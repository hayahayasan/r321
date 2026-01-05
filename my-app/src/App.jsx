import React, { useState, useEffect, useRef } from 'react';
import { Terminal, Send, Wifi, WifiOff, Fingerprint, AlertCircle, ArrowUpCircle, Lock, Trash2 } from 'lucide-react';

/**
 * 【重要】publicフォルダの参照ルール
 * public/icon.png   => "/icon.png"
 * public/haikei.png => "/haikei.png"
 */
const FAVICON_PATH = '/icon.png'; 
const BACKGROUND_IMAGE = '/haikei.png';
const TITLE_TEXT = 'The M5 Sotuken App';

const HEARTBEAT_INTERVAL = 3000;
const PING_FAIL_THRESHOLD = 3;
const WS_PORT = 65500;

export default function App() {
  const [messages, setMessages] = useState([]);
  const [inputCommand, setInputCommand] = useState('');
  const [status, setStatus] = useState('disconnected');
  const [isCookieDisabled, setIsCookieDisabled] = useState(false);
  const [m5Ip, setM5Ip] = useState(() => {
    try {
      const saved = localStorage.getItem('m5_console_last_ip');
      return saved !== null ? saved : '192.168.1.100';
    } catch (e) { return '192.168.1.100'; }
  });
  const [userId, setUserId] = useState('');
  const [isTabBlocked, setIsTabBlocked] = useState(false);
  
  const socketRef = useRef(null);
  const tabIdRef = useRef(typeof crypto !== 'undefined' && crypto.randomUUID ? crypto.randomUUID() : Math.random().toString(36).substring(2));
  const heartbeatTimerRef = useRef(null);
  const missedPingsRef = useRef(0);

  useEffect(() => {
    // Cookie/LocalStorageの実行可否チェック
    if (!navigator.cookieEnabled) {
      setIsCookieDisabled(true);
      return;
    }

    document.title = TITLE_TEXT;
    let link = document.querySelector("link[rel*='icon']");
    if (!link) {
      link = document.createElement('link');
      link.rel = 'icon';
      document.head.appendChild(link);
    }
    link.href = FAVICON_PATH;

    try {
      const savedId = localStorage.getItem('m5_console_user_id');
      if (savedId) {
        setUserId(savedId);
      } else {
        const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
        let result = '';
        for (let i = 0; i < 20; i++) result += chars.charAt(Math.floor(Math.random() * chars.length));
        localStorage.setItem('m5_console_user_id', result);
        setUserId(result);
      }

      localStorage.setItem('m5_console_active_tab', tabIdRef.current);
      const handleStorageChange = (e) => {
        if (e.key === 'm5_console_active_tab' && e.newValue !== tabIdRef.current) {
          setIsTabBlocked(true);
          disconnectWebSocket();
        }
      };
      window.addEventListener('storage', handleStorageChange);
      return () => window.removeEventListener('storage', handleStorageChange);
    } catch (e) {
      // LocalStorageが使えない場合（プライバシーモードなど）もエラー表示
      setIsCookieDisabled(true);
    }
  }, []);

  useEffect(() => {
    if (isCookieDisabled) return;
    try {
      localStorage.setItem('m5_console_last_ip', m5Ip);
    } catch (e) {}
  }, [m5Ip, isCookieDisabled]);

  const disconnectWebSocket = () => {
    if (heartbeatTimerRef.current) {
      clearInterval(heartbeatTimerRef.current);
      heartbeatTimerRef.current = null;
    }
    if (socketRef.current) {
      socketRef.current.close();
      socketRef.current = null;
    }
    setStatus('disconnected');
  };

  const connectWebSocket = () => {
    if (isTabBlocked || isCookieDisabled) return;
    
    disconnectWebSocket();

    const url = `ws://${m5Ip}:${WS_PORT}`;
    addLog('System', `Connecting to ${url}...`);
    setStatus('connecting');
    
    try {
      const socket = new WebSocket(url);
      
      socket.onopen = () => {
        setStatus('connected');
        addLog('System', `Connected to ${url}`);
        socket.send(`id:${userId}`);

        missedPingsRef.current = 0;
        heartbeatTimerRef.current = setInterval(() => {
          if (socket.readyState === WebSocket.OPEN) {
            if (missedPingsRef.current >= PING_FAIL_THRESHOLD) {
              addLog('Error', 'Timeout: No response from M5. Disconnecting...');
              disconnectWebSocket();
              return;
            }
            try {
              socket.send('ping:');
              missedPingsRef.current += 1;
            } catch (e) { console.error(e); }
          }
        }, HEARTBEAT_INTERVAL);
      };

      socket.onmessage = (event) => {
        missedPingsRef.current = 0;
        const data = event.data;
        if (data === 'pong' || data.startsWith('M5:resp:')) return;
        addLog('M5', data);
      };

      socket.onclose = () => {
        disconnectWebSocket();
      };

      socket.onerror = () => {
        addLog('Error', `Connection failed on port ${WS_PORT}.`);
        disconnectWebSocket();
        setStatus('error');
      };

      socketRef.current = socket;
    } catch (e) {
      addLog('Error', e.message);
      setStatus('error');
    }
  };

  const addLog = (sender, text) => {
    setMessages((prev) => {
      const newMsg = { id: Date.now() + Math.random(), sender, text, time: new Date().toLocaleTimeString() };
      return [newMsg, ...prev].slice(0, 50);
    });
  };

  const sendCommand = (e) => {
    e.preventDefault();
    if (!inputCommand.trim() || status !== 'connected') return;
    socketRef.current.send(inputCommand);
    addLog('You', inputCommand);
    setInputCommand('');
  };

  // Cookie/JS実行ができない環境向けの警告表示
  if (isCookieDisabled) {
    return (
      <div className="min-h-screen bg-slate-950 flex items-center justify-center p-6 text-white font-bold text-xl uppercase tracking-widest text-center">
        <div className="space-y-4">
          <AlertCircle className="mx-auto text-red-500" size={64} />
          <p className="animate-pulse">please relaunch with js and cookie enabled browser!</p>
        </div>
      </div>
    );
  }

  if (isTabBlocked) {
    return (
      <div className="min-h-screen bg-slate-900 flex items-center justify-center p-6 text-slate-100 text-center">
        <div className="bg-slate-800 p-8 rounded-2xl border border-red-500 shadow-2xl space-y-4">
          <Lock className="mx-auto text-red-500" size={48} />
          <h2 className="text-xl font-bold">Multiple Tabs Blocked</h2>
          <button onClick={() => window.location.reload()} className="bg-blue-600 px-6 py-2 rounded-lg">Reload</button>
        </div>
      </div>
    );
  }

  return (
    <div className="min-h-screen p-4 md:p-8 relative text-slate-100 font-sans">
      <noscript>
        <div className="fixed inset-0 z-[9999] bg-slate-950 flex items-center justify-center p-6 text-white font-bold text-xl uppercase text-center">
          please relaunch with js enabled browser!
        </div>
      </noscript>

      <style>{`
        body { 
          margin: 0; 
          background-color: #0f172a; 
          background-image: url('${BACKGROUND_IMAGE}'); 
          background-size: cover; 
          background-attachment: fixed; 
          background-position: center;
        }
        .scrollbar-custom::-webkit-scrollbar { width: 6px; }
        .scrollbar-custom::-webkit-scrollbar-thumb { background: #334155; border-radius: 10px; }
      `}</style>
      <div className="fixed inset-0 bg-slate-900/80 -z-10" />

      <div className="max-w-3xl mx-auto space-y-6">
        <header className="bg-slate-800/90 backdrop-blur-md p-6 rounded-2xl border border-slate-700 shadow-xl space-y-4">
          <div className="flex flex-col md:flex-row justify-between items-center gap-4">
            <h1 className="text-2xl font-bold text-blue-400 flex items-center gap-2"><Wifi /> M5 Console</h1>
            <div className="flex gap-2">
              <input value={m5Ip} onChange={e => setM5Ip(e.target.value)} className="bg-slate-900 border border-slate-600 rounded px-3 py-1 text-sm text-white w-36 focus:outline-none focus:ring-1 focus:ring-blue-500" />
              <button onClick={status === 'connected' ? disconnectWebSocket : connectWebSocket} className={`${status === 'connected' ? 'bg-red-500' : 'bg-blue-600'} px-4 py-2 rounded font-bold text-sm shadow-lg hover:opacity-90 transition-opacity`}>
                {status === 'connected' ? 'Disconnect' : 'Connect'}
              </button>
            </div>
          </div>
          <div className="bg-slate-900/50 p-3 rounded-xl border border-slate-700/50 flex items-center justify-between">
            <div className="flex items-center gap-2 text-sm font-mono text-indigo-300">
              <Fingerprint size={16} /> {userId || 'Generating...'}
            </div>
            <span className="text-[10px] text-slate-600 font-bold italic">PORT: {WS_PORT}</span>
          </div>
        </header>

        <form onSubmit={sendCommand} className="relative">
          <input value={inputCommand} onChange={e => setInputCommand(e.target.value)} disabled={status !== 'connected'} placeholder="Enter command..." className="w-full bg-slate-800/90 border border-slate-700 rounded-xl p-4 pr-12 text-white shadow-lg focus:ring-2 focus:ring-blue-500 outline-none" />
          <button type="submit" className="absolute right-4 top-1/2 -translate-y-1/2 text-blue-500 hover:text-blue-400 transition-colors"><Send size={24} /></button>
        </form>

        <div className="flex flex-col gap-2">
          <div className="p-3 bg-slate-700/30 border-2 border-slate-600/60 rounded shadow-[inset_0_0_10px_rgba(0,0,0,0.5)]">
            <div className="bg-slate-950/90 h-[400px] flex flex-col overflow-hidden border border-slate-800">
              <div className="bg-slate-900 px-4 py-2 border-b border-slate-800 flex justify-between items-center text-[10px] font-mono text-slate-400">
                <span className="flex items-center gap-1"><Terminal size={12}/> TERMINAL_LOG</span>
                <span className={status === 'connected' ? 'text-green-500' : 'text-red-500 animate-pulse'}>{status.toUpperCase()}</span>
              </div>
              <div className="flex-1 overflow-y-auto p-4 space-y-1 font-mono text-xs scrollbar-custom">
                {messages.length === 0 && <div className="text-slate-800 italic text-center mt-20 font-sans">No logs yet.</div>}
                {messages.map(msg => (
                  <div key={msg.id} className="flex gap-2 border-b border-slate-900 pb-1 opacity-90 animate-in fade-in duration-300">
                    <span className="text-slate-600">[{msg.time}]</span>
                    <span className={msg.sender === 'M5' ? 'text-green-500' : msg.sender === 'System' ? 'text-indigo-400' : 'text-blue-400'}>{msg.sender}:</span>
                    <span className="text-slate-300 break-all">{msg.text}</span>
                  </div>
                ))}
              </div>
            </div>
          </div>
          <div className="flex justify-end">
            <button onClick={() => setMessages([])} className="flex items-center gap-1 text-[10px] font-bold text-slate-500 hover:text-red-400 px-3 py-2 bg-slate-800 rounded border border-slate-700 shadow-sm transition-colors">
              <Trash2 size={12}/> CLEAR HISTORY
            </button>
          </div>
        </div>

        <footer className="bg-slate-900/60 p-4 rounded-xl border border-slate-800 text-[10px] text-slate-500 text-center space-y-2">
          <p className="text-slate-300 font-bold italic">接続はping応答が3回連続で途絶えると即座に切断されます</p>
          <div className="grid grid-cols-2 gap-2 opacity-70">
             <p>dataload:(数):SD読込</p>
             <p>datasave:(テ),(数):SD書込</p>
             <p>sendme:(テ):反復送信</p>
             <p>test: :テスト信号</p>
             <p>sendo:(テ),(ID):特定送信</p>
             <p>list: :接続リスト</p>
          </div>
        </footer>
      </div>
    </div>
  );
}