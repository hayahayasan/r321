import React, { useState, useEffect, useRef } from 'react';
import { Terminal, Send, Wifi, WifiOff, UserPlus, Fingerprint, AlertCircle, ArrowUpCircle, Lock, FileText, Download, Trash2, Image as ImageIcon, Loader2 } from 'lucide-react';

/**
 * M5Stack WebSocket Client App
 * - WebSocket接続 (Port 80)
 * - タブの多重起動防止機能
 * - ログ表示 (最新50件)
 * - ファイル受信機能 (バイナリ受信対応)
 * - 自動切断・ハートビート機能 (10秒タイムアウト)
 * - ping: 送信に対し、pong または M5:resp:<20桁のブラウザID> を受信で生存確認
 */

const HEARTBEAT_INTERVAL = 3000; // 3秒ごとにPing送信
const CONNECTION_TIMEOUT = 10000; // 10秒間受信がなければ切断

export default function App() {
  const [messages, setMessages] = useState([]);
  const [inputCommand, setInputCommand] = useState('');
  const [status, setStatus] = useState('disconnected');
  
  // 初期値の段階で localStorage から読み込む
  const [m5Ip, setM5Ip] = useState(() => {
    const saved = localStorage.getItem('m5_console_last_ip');
    return saved !== null ? saved : '192.168.1.100';
  });

  const [userId, setUserId] = useState('');
  const [isTabBlocked, setIsTabBlocked] = useState(false);
  
  // ファイル受信用のState
  const [receivedFiles, setReceivedFiles] = useState([]);
  const [receivingState, setReceivingState] = useState(null); 
  
  const socketRef = useRef(null);
  const fileChunksRef = useRef([]); 
  const tabIdRef = useRef(typeof crypto !== 'undefined' && crypto.randomUUID ? crypto.randomUUID() : Math.random().toString(36).substring(2));
  
  // ハートビート管理用Ref
  const heartbeatTimerRef = useRef(null);
  const lastMessageTimeRef = useRef(0);

  // 初回起動時のデータ復元 (ID) および 多重起動チェック
  useEffect(() => {
    const savedId = localStorage.getItem('m5_console_user_id');
    if (savedId) {
      setUserId(savedId);
    } else {
      const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
      let result = '';
      for (let i = 0; i < 20; i++) {
        result += chars.charAt(Math.floor(Math.random() * chars.length));
      }
      localStorage.setItem('m5_console_user_id', result);
      setUserId(result);
    }

    localStorage.setItem('m5_console_active_tab', tabIdRef.current);

    const handleStorageChange = (e) => {
      if (e.key === 'm5_console_active_tab' && e.newValue !== tabIdRef.current) {
        setIsTabBlocked(true);
        if (socketRef.current) {
          socketRef.current.close();
        }
      }
    };

    window.addEventListener('storage', handleStorageChange);
    return () => window.removeEventListener('storage', handleStorageChange);
  }, []);

  // IPアドレスが変更されたらlocalStorageに保存
  useEffect(() => {
    localStorage.setItem('m5_console_last_ip', m5Ip);
  }, [m5Ip]);

  // WebSocket接続処理
  const connectWebSocket = () => {
    if (isTabBlocked) return;
    
    // 既存の接続とタイマーをクリア
    if (socketRef.current) socketRef.current.close();
    if (heartbeatTimerRef.current) clearInterval(heartbeatTimerRef.current);

    const url = `ws://${m5Ip}:80`;
    addLog('System', `Attempting to connect to ${url}...`);
    
    try {
      const socket = new WebSocket(url);
      
      socket.binaryType = 'arraybuffer';

      socket.onopen = () => {
        setStatus('connected');
        addLog('System', `Connected successfully to ${url}`);
        
        const idMessage = `id:${userId}`;
        socket.send(idMessage);
        addLog('System', `Auto-sent device ID: ${idMessage}`);

        // ハートビート監視開始
        lastMessageTimeRef.current = Date.now();
        heartbeatTimerRef.current = setInterval(() => {
          // ソケットが開いていれば Ping を送る
          if (socket.readyState === WebSocket.OPEN) {
            try {
              socket.send('ping:');
            } catch (e) {
              console.error('Ping send failed', e);
            }
          }
          
          // 最終受信時刻からの経過時間をチェック
          if (Date.now() - lastMessageTimeRef.current > CONNECTION_TIMEOUT) {
            addLog('Error', 'Connection timed out (no response). Disconnecting...');
            
            // 強制的に切断処理を行う
            socket.close();
            setStatus('disconnected');
            
            if (heartbeatTimerRef.current) {
              clearInterval(heartbeatTimerRef.current);
              heartbeatTimerRef.current = null;
            }
          }
        }, HEARTBEAT_INTERVAL);
      };

      socket.onmessage = (event) => {
        // データを受信したら時刻を更新（生存確認）
        lastMessageTimeRef.current = Date.now();
        
        const data = event.data;

        if (data instanceof ArrayBuffer) {
          if (fileChunksRef.current) {
            fileChunksRef.current.push(data);
            setReceivingState(prev => prev ? { ...prev, chunks: prev.chunks + 1 } : null);
          }
          return;
        }

        if (typeof data === 'string') {
          // 'pong' または 'M5:resp:<20桁のBrowserID>' で始まるメッセージは、
          // 生存確認として扱い、ログには表示しない
          if (data === 'pong' || data.startsWith('M5:resp:')) {
            // ここで return することでログ出力をスキップします
            // (lastMessageTimeRef は上で更新済みなので接続は維持されます)
            return;
          }

          if (data.startsWith('file_start:')) {
            const filename = data.split(':')[1] || `file_${Date.now()}.bin`;
            fileChunksRef.current = []; 
            setReceivingState({ name: filename, chunks: 0 });
            addLog('System', `Start receiving file: ${filename}`);
          }
          else if (data === 'file_end') {
            if (receivingState && fileChunksRef.current.length > 0) {
              finishFileReception(receivingState.name);
            } else {
              addLog('Error', 'Received file_end signal but no data captured.');
            }
          }
          else {
            addLog('M5', data); // 送信者名を 'M5' に設定
          }
        }
      };

      socket.onclose = (event) => {
        setStatus('disconnected');
        setReceivingState(null);
        
        // タイマー停止
        if (heartbeatTimerRef.current) {
          clearInterval(heartbeatTimerRef.current);
          heartbeatTimerRef.current = null;
        }

        const reason = event.code === 1000 ? 'Normal Closure' : `Error Code: ${event.code}`;
        const detail = event.reason ? ` (${event.reason})` : '';
        addLog('System', `Disconnected. Reason: ${reason}${detail}`);
      };

      socket.onerror = (error) => {
        setStatus('error');
        addLog('Error', 'WebSocket connection failed. Check IP address and Port 80.');
      };

      socketRef.current = socket;
    } catch (e) {
      setStatus('error');
      addLog('Error', `Fatal error: ${e.message}`);
    }
  };

  const finishFileReception = (filename) => {
    try {
      const blob = new Blob(fileChunksRef.current);
      const url = URL.createObjectURL(blob);
      const type = filename.match(/\.(jpg|jpeg|png|gif)$/i) ? 'image' : 'file';

      const newFile = {
        id: Date.now(),
        name: filename,
        size: blob.size,
        url: url,
        type: type,
        timestamp: new Date().toLocaleTimeString()
      };

      setReceivedFiles(prev => [newFile, ...prev]);
      addLog('System', `File received: ${filename} (${formatBytes(blob.size)})`);
    } catch (e) {
      addLog('Error', `Failed to process file: ${e.message}`);
    } finally {
      fileChunksRef.current = [];
      setReceivingState(null);
    }
  };

  const disconnectWebSocket = () => {
    if (heartbeatTimerRef.current) {
      clearInterval(heartbeatTimerRef.current);
      heartbeatTimerRef.current = null;
    }
    socketRef.current?.close();
  };

  const addLog = (sender, text) => {
    setMessages((prev) => {
      const newMsg = {
        id: Date.now() + Math.random(),
        sender,
        text,
        time: new Date().toLocaleTimeString()
      };
      return [newMsg, ...prev].slice(0, 50);
    });
  };

  const sendCommand = (e) => {
    e.preventDefault();
    if (!inputCommand.trim() || status !== 'connected' || isTabBlocked) return;

    socketRef.current.send(inputCommand);
    addLog('You', inputCommand);
    setInputCommand('');
  };

  const handleLogin = () => {
    if (status !== 'connected' || isTabBlocked) return;
    const payload = 'login';
    socketRef.current.send(payload);
    addLog('You', payload);
  };

  const deleteFile = (id) => {
    setReceivedFiles(prev => prev.filter(f => f.id !== id));
  };

  const formatBytes = (bytes, decimals = 2) => {
    if (!+bytes) return '0 Bytes';
    const k = 1024;
    const dm = decimals < 0 ? 0 : decimals;
    const sizes = ['Bytes', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return `${parseFloat((bytes / Math.pow(k, i)).toFixed(dm))} ${sizes[i]}`;
  };

  if (isTabBlocked) {
    return (
      <div className="min-h-screen bg-slate-900 flex items-center justify-center p-6 text-center font-sans">
        <div className="max-w-md bg-slate-800 p-8 rounded-3xl shadow-2xl border border-red-500/50 space-y-6">
          <div className="bg-red-500/10 w-20 h-20 rounded-full flex items-center justify-center mx-auto text-red-500">
            <Lock size={40} />
          </div>
          <div className="space-y-2">
            <h2 className="text-2xl font-bold text-slate-100">Multiple Tabs Detected</h2>
            <p className="text-slate-400 text-sm">
              別のタブでコンソールが起動されました。セッションの競合を防ぐため、このタブの接続は切断されました。
            </p>
          </div>
          <button 
            onClick={() => window.location.reload()}
            className="w-full bg-blue-600 hover:bg-blue-700 text-white font-bold py-3 rounded-xl transition-all"
          >
            このタブを再アクティブ化する
          </button>
        </div>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-slate-900 text-slate-100 p-4 md:p-8 font-sans">
      <div className="max-w-3xl mx-auto space-y-6">
        
        {/* Header */}
        <header className="flex flex-col gap-4 bg-slate-800 p-6 rounded-2xl shadow-xl border border-slate-700">
          <div className="flex flex-col md:flex-row md:items-center justify-between gap-4">
            <div>
              <h1 className="text-2xl font-bold flex items-center gap-2 text-blue-400">
                <Wifi size={28} /> M5Stack Console
              </h1>
              <p className="text-slate-400 text-sm mt-1">WebSocket Controller (Port 80)</p>
            </div>
            
            <div className="flex items-center gap-2">
              <input 
                type="text" 
                value={m5Ip}
                onChange={(e) => setM5Ip(e.target.value)}
                placeholder="M5Stack IP"
                className="bg-slate-900 border border-slate-600 rounded-lg px-3 py-2 text-sm focus:outline-none focus:ring-2 focus:ring-blue-500 w-40 transition-colors"
              />
              {status === 'connected' ? (
                <button 
                  onClick={disconnectWebSocket}
                  className="bg-red-500 hover:bg-red-600 px-4 py-2 rounded-lg text-sm font-bold transition-colors flex items-center gap-2 whitespace-nowrap shadow-lg shadow-red-900/20"
                >
                  <WifiOff size={16} /> Disconnect
                </button>
              ) : (
                <button 
                  onClick={connectWebSocket}
                  className="bg-blue-600 hover:bg-blue-700 px-4 py-2 rounded-lg text-sm font-bold transition-colors flex items-center gap-2 whitespace-nowrap shadow-lg shadow-blue-900/20"
                >
                  <Wifi size={16} /> Connect
                </button>
              )}
            </div>
          </div>

          {/* User ID Section */}
          <div className="flex items-center justify-between bg-slate-900/50 p-3 rounded-xl border border-slate-700/50">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-indigo-500/20 rounded-lg text-indigo-400">
                <Fingerprint size={20} />
              </div>
              <div>
                <div className="text-[10px] uppercase tracking-widest text-slate-500 font-bold">Your Device ID (Permanent)</div>
                <div className="text-sm font-mono text-indigo-300">{userId || 'Generating...'}</div>
              </div>
            </div>
            <div className="text-[10px] text-slate-600 font-bold px-2">READ ONLY</div>
          </div>
        </header>

        {/* File Receiving Status */}
        {receivingState && (
          <div className="bg-indigo-900/30 border border-indigo-500/30 p-4 rounded-xl flex items-center gap-3 animate-pulse">
            <Loader2 className="animate-spin text-indigo-400" size={20} />
            <div className="flex-1">
              <div className="text-sm font-bold text-indigo-300">Receiving: {receivingState.name}</div>
              <div className="text-xs text-indigo-400/70">Processing chunk #{receivingState.chunks}...</div>
            </div>
          </div>
        )}

        {/* Received Files List */}
        {receivedFiles.length > 0 && (
          <div className="grid grid-cols-1 md:grid-cols-2 gap-3">
            {receivedFiles.map(file => (
              <div key={file.id} className="bg-slate-800 border border-slate-700 p-3 rounded-xl flex items-center gap-3 shadow-lg overflow-hidden group">
                <div className="w-12 h-12 bg-slate-900 rounded-lg flex items-center justify-center shrink-0 border border-slate-700">
                  {file.type === 'image' ? (
                    <img src={file.url} alt="preview" className="w-full h-full object-cover rounded-lg" />
                  ) : (
                    <FileText size={24} className="text-slate-500" />
                  )}
                </div>
                <div className="flex-1 min-w-0">
                  <div className="text-sm font-bold truncate text-slate-200" title={file.name}>{file.name}</div>
                  <div className="text-xs text-slate-500">{formatBytes(file.size)} • {file.timestamp}</div>
                </div>
                <div className="flex items-center gap-1">
                  <a 
                    href={file.url} 
                    download={file.name}
                    className="p-2 hover:bg-slate-700 rounded-lg text-blue-400 transition-colors"
                    title="Download"
                  >
                    <Download size={18} />
                  </a>
                  <button 
                    onClick={() => deleteFile(file.id)}
                    className="p-2 hover:bg-slate-700 rounded-lg text-red-400 transition-colors"
                    title="Delete"
                  >
                    <Trash2 size={18} />
                  </button>
                </div>
              </div>
            ))}
          </div>
        )}

        {/* Logs and Input Area */}
        <div className="space-y-4">
          {/* Status Indicator */}
          <div className="flex items-center justify-between px-2">
            <div className="flex items-center gap-2">
              <div className={`w-3 h-3 rounded-full ${status === 'connected' ? 'bg-green-500 animate-pulse' : status === 'error' ? 'bg-orange-500' : 'bg-red-500'}`} />
              <span className="text-sm font-medium uppercase tracking-wider text-slate-400">{status}</span>
            </div>
            {status === 'error' && (
              <div className="flex items-center gap-1 text-orange-400 text-xs animate-bounce">
                <AlertCircle size={14} /> Check console or logs for codes
              </div>
            )}
          </div>

          <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
            <form onSubmit={sendCommand} className="md:col-span-3 relative">
              <input 
                type="text"
                value={inputCommand}
                onChange={(e) => setInputCommand(e.target.value)}
                disabled={status !== 'connected'}
                placeholder={status === 'connected' ? "Enter command..." : "Please connect first..."}
                className="w-full bg-slate-800 border border-slate-700 rounded-xl px-4 py-4 pr-12 focus:outline-none focus:ring-2 focus:ring-blue-500 disabled:opacity-50 transition-all shadow-lg"
              />
              <button 
                type="submit"
                disabled={status !== 'connected'}
                className="absolute right-3 top-1/2 -translate-y-1/2 text-blue-500 hover:text-blue-400 disabled:text-slate-600 transition-colors"
              >
                <Send size={24} />
              </button>
            </form>

            <button 
              onClick={handleLogin}
              disabled={status !== 'connected'}
              className="bg-indigo-600 hover:bg-indigo-700 disabled:bg-slate-700 rounded-xl px-4 py-4 font-bold flex items-center justify-center gap-2 transition-all shadow-lg active:scale-95"
            >
              <UserPlus size={20} /> Login
            </button>
          </div>

          {/* Messages Log */}
          <div className="bg-slate-950 rounded-2xl border border-slate-800 h-[300px] flex flex-col overflow-hidden shadow-inner relative">
            <div className="bg-slate-800/50 px-4 py-2 border-b border-slate-800 flex items-center justify-between text-slate-400 text-xs font-mono sticky top-0 z-10 backdrop-blur-sm">
              <div className="flex items-center gap-2">
                <Terminal size={14} /> TERMINAL_OUTPUT (Max 50 lines)
              </div>
              <div className="flex items-center gap-1 text-[10px] bg-slate-800 px-2 py-0.5 rounded">
                <ArrowUpCircle size={10} /> Newest First
              </div>
            </div>
            <div className="flex-1 overflow-y-auto p-4 space-y-2 font-mono text-sm">
              {messages.length === 0 && (
                <div className="text-slate-600 italic text-center mt-10">No logs yet.</div>
              )}
              {messages.map((msg) => (
                <div key={msg.id} className="flex gap-2 animate-in fade-in slide-in-from-top-2 duration-200 border-b border-slate-800/30 pb-1 last:border-0">
                  <span className="text-slate-500 shrink-0">[{msg.time}]</span>
                  <span className={`font-bold shrink-0 ${
                    msg.sender === 'M5' ? 'text-green-400' : 
                    msg.sender === 'You' ? 'text-blue-400' : 
                    msg.sender === 'System' ? 'text-indigo-400' : 
                    msg.sender === 'Error' ? 'text-red-400' : 'text-amber-400'
                  }`}>
                    {msg.sender}:
                  </span>
                  <span className={`text-slate-300 break-all ${msg.sender === 'Error' ? 'bg-red-900/20 px-1 rounded' : ''}`}>
                    {msg.text}
                  </span>
                </div>
              ))}
            </div>
          </div>
        </div>

        <footer className="text-center text-slate-500 text-xs py-4 flex flex-col gap-1">
          <p>Auto-disconnects if no data is received for 10s (sends 'ping:' every 3s).</p>
          <p>File Transfer Protocol: "file_start:name" → [Binary Data] → "file_end"</p>
        </footer>
      </div>
    </div>
  );
}