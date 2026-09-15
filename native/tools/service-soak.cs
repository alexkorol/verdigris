using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Net.WebSockets;
using System.Text;
using System.Threading;
using System.Web.Script.Serialization;

// Test-only TCP relay: delay each ordered read chunk before forwarding it.
// This models directional delay/jitter, not Internet routing, packet loss or TLS.
public static class VerdigrisSoak {
    public static string Stage = "starting";
    static JavaScriptSerializer Serializer() { return new JavaScriptSerializer { MaxJsonLength = 2097152, RecursionLimit = 64 }; }
    static void Check(bool value, string message) { if (!value) throw new InvalidOperationException(message); }
    static Dictionary<string, object> Obj(object value) { return (Dictionary<string, object>)value; }
    static string Str(Dictionary<string, object> value, string key) { return Convert.ToString(value[key]); }
    static Dictionary<string, object> D(params object[] pairs) { var d = new Dictionary<string, object>(); for (int i=0;i<pairs.Length;i+=2) d[(string)pairs[i]]=pairs[i+1]; return d; }
    sealed class Relay : IDisposable {
        readonly TcpListener listener = new TcpListener(IPAddress.Loopback, 0);
        readonly int destination, seed; readonly Thread accept;
        TcpClient incoming, outgoing; volatile bool stopped;
        public int Port; public long ForwardChunks, ReverseChunks, ForwardDelayMs, ReverseDelayMs;
        public Relay(int port, int randomSeed) { destination=port; seed=randomSeed; listener.Start(); Port=((IPEndPoint)listener.LocalEndpoint).Port; accept=new Thread(Accept); accept.IsBackground=true; accept.Start(); }
        void Accept() { try { incoming=listener.AcceptTcpClient(); incoming.NoDelay=true; outgoing=new TcpClient(); outgoing.NoDelay=true; outgoing.Connect(IPAddress.Loopback,destination); var reverse=new Thread(delegate(){ Pump(outgoing,incoming,false); }); reverse.IsBackground=true; reverse.Start(); Pump(incoming,outgoing,true); reverse.Join(2000); } catch (Exception) { if (!stopped) DisposeSockets(); } }
        void Pump(TcpClient source, TcpClient target, bool forward) { try { var random=new Random(seed+(forward?0:10000)); var bytes=new byte[65536]; while (!stopped) { int count=source.GetStream().Read(bytes,0,bytes.Length); if(count==0) break; int delay=20+random.Next(16); Thread.Sleep(delay); target.GetStream().Write(bytes,0,count); if(forward) { Interlocked.Increment(ref ForwardChunks); Interlocked.Add(ref ForwardDelayMs,delay); } else { Interlocked.Increment(ref ReverseChunks); Interlocked.Add(ref ReverseDelayMs,delay); } } } catch (Exception) {} finally { DisposeSockets(); } }
        void DisposeSockets() { try { if(incoming!=null) incoming.Close(); } catch {} try { if(outgoing!=null) outgoing.Close(); } catch {} }
        public void Dispose() { stopped=true; listener.Stop(); DisposeSockets(); accept.Join(2000); }
    }
    sealed class Peer : IDisposable {
        public readonly Relay Relay; readonly ClientWebSocket socket=new ClientWebSocket(); readonly object gate=new object();
        readonly List<Dictionary<string,object>> queue=new List<Dictionary<string,object>>(); readonly Thread reader;
        Exception failure; bool disposed; long commandSequence, movementSequence;
        public string Account, Token, Epoch, Actor, Scene; public long ActorFrames; public Dictionary<string,object> Actors;
        public Peer(int port,int seed) { Relay=new Relay(port,seed); using(var cancel=new CancellationTokenSource(5000)) socket.ConnectAsync(new Uri("ws://127.0.0.1:"+Relay.Port+"/game"),cancel.Token).GetAwaiter().GetResult(); reader=new Thread(Read); reader.IsBackground=true; reader.Start(); }
        void Read() { try { var json=Serializer(); var buffer=new byte[16384]; while(true) { using(var message=new MemoryStream()) { WebSocketReceiveResult received; do { received=socket.ReceiveAsync(new ArraySegment<byte>(buffer),CancellationToken.None).GetAwaiter().GetResult(); Check(received.MessageType==WebSocketMessageType.Text,"Peer closed or received non-text frame."); message.Write(buffer,0,received.Count); Check(message.Length<=1048576,"Receive bound exceeded."); } while(!received.EndOfMessage); var envelope=json.Deserialize<Dictionary<string,object>>(Encoding.UTF8.GetString(message.ToArray())); string name=Str(envelope,"event"); lock(gate) { if(name=="world:actors") { Actors=Obj(envelope["data"]); ActorFrames++; } else if(name=="service:result" || name=="service:authenticated" || name=="service:rejected" || name=="service:pong" || name=="chronicles:state" || name=="world:snapshot" || name=="party:invited" || name=="party:error") { Check(queue.Count<128,"Test response queue exceeded bound."); queue.Add(envelope); } Monitor.PulseAll(gate); } } } } catch(Exception e) { lock(gate) { failure=e; Monitor.PulseAll(gate); } } }
        public void Send(string name, Dictionary<string,object> data) { var bytes=Encoding.UTF8.GetBytes(Serializer().Serialize(D("event",name,"data",data))); using(var cancel=new CancellationTokenSource(5000)) socket.SendAsync(new ArraySegment<byte>(bytes),WebSocketMessageType.Text,true,cancel.Token).GetAwaiter().GetResult(); }
        public Dictionary<string,object> Until(string name) { var clock=Stopwatch.StartNew(); lock(gate) { while(clock.ElapsedMilliseconds<5000) { for(int i=0;i<queue.Count;i++) { string found=Str(queue[i],"event"); if(found=="party:error" || found=="service:rejected") throw new InvalidOperationException("Service rejected ordinary test operation: "+found); if(found==name) { var data=Obj(queue[i]["data"]); queue.RemoveAt(i); return data; } } if(failure!=null) throw new InvalidOperationException("Peer reader failed",failure); Monitor.Wait(gate,100); } } throw new TimeoutException("Waiting for "+name); }
        public void Authenticate(string credential,bool enroll) { Send("service:authenticate",D("credential",credential,"enroll",enroll,"protocolVersion",1)); var auth=Until("service:authenticated"); Account=Str(auth,"accountId"); Token=Str(auth,"token"); Epoch=Str(auth,"commandEpoch"); Snapshot(); }
        public void Command(string name,Dictionary<string,object> data) { data["commandEpoch"]=Epoch; data["commandSequence"]=++commandSequence; Send(name,data); Check(Str(Until("service:result"),"status")=="processed","Ordinary command not processed."); }
        public Dictionary<string,object> Snapshot() { Send("service:ping",D()); Until("service:pong"); lock(gate) queue.RemoveAll(delegate(Dictionary<string,object> e){return Str(e,"event")=="world:snapshot";}); Send("world:snapshot",D("includeMap",false)); var state=Obj(Until("world:snapshot")["state"]); Actor=Str(state,"uuid"); Scene=Str(state,"sceneId"); return state; }
        public double GameplaySample(string direction) { var clock=Stopwatch.StartNew(); Send("player:move",D("direction",direction,"sequence",++movementSequence,"sceneId",Scene,"actingActorId",Actor)); Send("world:snapshot",D("includeMap",false)); var state=Obj(Until("world:snapshot")["state"]); Check(Str(state,"sceneId")==Scene && Str(state,"uuid")==Actor && Str(state,"lifecycle")=="alive","Actor lost its living instance during soak."); return clock.Elapsed.TotalMilliseconds; }
        public void Create(int index) { Command("chronicles:house:found",D("name","Soak House "+index)); var founded=Until("chronicles:state"); var houses=(System.Collections.IList)Obj(founded["chronicle"])["houses"]; string house=Str(Obj(houses[0]),"id"); Command("chronicles:scion:create",D("houseId",house,"name","Soak Scion "+index,"appearance","male")); string scion=Str(Until("chronicles:state"),"createdScionId"); Command("chronicles:scion:set-out",D("scionId",scion)); Check(Str(Snapshot(),"lifecycle")=="alive","Ordinary Scion admission failed."); }
        public void CheckRoster(string ally) { lock(gate) { Check(Actors!=null && Str(Actors,"sceneId")==Scene,"Missing current instance presence."); var actors=(System.Collections.IList)Actors["actors"]; Check(actors.Count==2,"Instance roster leaked or lost an actor."); bool self=false,peer=false; foreach(var item in actors) { string id=Str(Obj(item),"uuid"); self|=id==Actor; peer|=id==ally; } Check(self&&peer,"Instance roster identities mismatch."); } }
        public void Dispose() { if(disposed)return; disposed=true; socket.Abort(); Relay.Dispose(); reader.Join(2000); socket.Dispose(); }
    }
    sealed class Samples : IDisposable {
        readonly object gate=new object(); readonly int pid; readonly Timer timer; public long MaxPrivate,MaxWorking; public int MaxThreads,MaxHandles;
        public Samples(int id) { pid=id; Take(null); timer=new Timer(Take,null,50,50); }
        void Take(object ignored) { try { using(var p=Process.GetProcessById(pid)) lock(gate) { MaxPrivate=Math.Max(MaxPrivate,p.PrivateMemorySize64); MaxWorking=Math.Max(MaxWorking,p.WorkingSet64); MaxThreads=Math.Max(MaxThreads,p.Threads.Count); MaxHandles=Math.Max(MaxHandles,p.HandleCount); } } catch(InvalidOperationException) {} catch(ArgumentException) {} }
        public void Dispose() { timer.Dispose(); }
    }
    static Dictionary<string,object> Resources(int pid) { using(var p=Process.GetProcessById(pid)) return D("privateBytes",p.PrivateMemorySize64,"workingSetBytes",p.WorkingSet64,"threads",p.Threads.Count,"handles",p.HandleCount,"cpuMilliseconds",p.TotalProcessorTime.TotalMilliseconds); }
    static void Party(Peer a,Peer b) { a.Command("party:create",D()); a.Command("party:invite",D("actorId",b.Actor)); string party=Str(Obj(b.Until("party:invited")["invite"]),"partyId"); b.Command("party:invite:accept",D("partyId",party)); a.Command("party:ready",D("ready",true)); b.Command("party:ready",D("ready",true)); a.Command("party:startInstance",D()); a.Snapshot(); b.Snapshot(); Check(a.Scene.StartsWith("instance:")&&a.Scene==b.Scene,"Two-player expedition not admitted."); }
    public static string Run(int port,int pid,string[] credentials,int seconds) {
        Check(seconds>=10 && seconds<=600,"Soak duration must be 10..600 seconds.");
        var peers=new Peer[4]; var relays=new List<Relay>(); var times=new List<double>(); var result=D("clients",4,"instances",2,"requestedSoakSeconds",seconds,"seed",20260914,"oneWayChunkDelayMinimumMs",20,"oneWayChunkDelayMaximumMs",35,"nominalRoundTripInjectedDelayRangeMs",new int[]{40,70},"topology","Local ordered TCP relays, one per client; independently seeded directional per-read-chunk delay. No Internet or TLS coverage.");
        var baseline=Resources(pid); result["beforeAdmission"]=baseline;
        using(var samples=new Samples(pid)) try {
            Stage="ordinary four-account setup";
            for(int i=0;i<4;i++) { peers[i]=new Peer(port,20260914+i); relays.Add(peers[i].Relay); peers[i].Authenticate(credentials[i],true); peers[i].Create(i); }
            Stage="two independent parties"; Party(peers[0],peers[1]); Party(peers[2],peers[3]); Check(peers[0].Scene!=peers[2].Scene,"Independent parties shared instance identity.");
            result["instanceIds"]=new string[]{peers[0].Scene,peers[2].Scene}; result["soakStart"]=Resources(pid);
            var clock=Stopwatch.StartNew(); bool interrupted=false; long rounds=0; int moves=0;
            Stage="four-client two-instance soak";
            while(clock.Elapsed.TotalSeconds<seconds) {
                if(!interrupted && clock.Elapsed.TotalSeconds>=seconds/2.0) {
                    Stage="two-second connection interruption"; string account=peers[1].Account,token=peers[1].Token,actor=peers[1].Actor,scene=peers[1].Scene,epoch=peers[1].Epoch;
                    var recovery=Stopwatch.StartNew(); peers[1].Dispose();
                    // The remaining three clients continue ordinary snapshot traffic during loss.
                    var loss=Stopwatch.StartNew(); while(loss.ElapsedMilliseconds<2000) { foreach(int i in new int[]{0,2,3}) times.Add(peers[i].GameplaySample("")); Thread.Sleep(50); }
                    result["interruptionMilliseconds"]=loss.Elapsed.TotalMilliseconds;
                    peers[1]=new Peer(port,20261014); relays.Add(peers[1].Relay); peers[1].Authenticate(token,false);
                    Check(peers[1].Account==account && peers[1].Actor==actor && peers[1].Scene==scene && peers[1].Epoch!=epoch,"Reconnect did not preserve reserved account/actor/instance or rotate epoch.");
                    result["interruptionToAuthenticatedSnapshotMs"]=recovery.Elapsed.TotalMilliseconds; interrupted=true; Stage="four-client two-instance soak resumed";
                }
                for(int i=0;i<4;i++) { times.Add(peers[i].GameplaySample("")); peers[i].CheckRoster(peers[i^1].Actor); }
                // Bounded ordinary movement pulses stay in the protected entry clearing.
                if(rounds%8==0) { for(int i=0;i<4;i++) { var before=peers[i].Snapshot(); double x=Convert.ToDouble(before["x"]); peers[i].GameplaySample(x>6.25?"left":"right"); Thread.Sleep(100); peers[i].GameplaySample(""); var after=peers[i].Snapshot(); if(Math.Abs(Convert.ToDouble(after["x"])-x)>0.001) moves++; Check(Convert.ToDouble(after["x"])>=2&&Convert.ToDouble(after["x"])<=10,"Movement left protected clearing."); } }
                rounds++; Thread.Sleep(100);
            }
            Check(interrupted && moves>=4,"Soak did not exercise reconnect and movement.");
            result["actualSoakMilliseconds"]=clock.Elapsed.TotalMilliseconds; result["rounds"]=rounds; result["observedMovementChanges"]=moves; result["soakEnd"]=Resources(pid);
            var final=new List<object>(); long actorFrames=0; foreach(var peer in peers) { var state=peer.Snapshot(); Check(Convert.ToDouble(Obj(state["hp"])["current"])>0,"Soak actor died."); final.Add(D("actorId",peer.Actor,"sceneId",peer.Scene,"x",state["x"],"y",state["y"],"hp",state["hp"],"presenceFrames",peer.ActorFrames)); actorFrames+=peer.ActorFrames; } result["finalActors"]=final; result["presenceFrames"]=actorFrames;
            times.Sort(); result["gameplaySnapshotRoundTripSamples"]=times.Count; result["gameplaySnapshotRoundTripMedianMs"]=times[times.Count/2]; result["gameplaySnapshotRoundTripP95Ms"]=times[(int)((times.Count-1)*0.95)]; result["gameplaySnapshotRoundTripMaxMs"]=times[times.Count-1]; Check(times[times.Count-1]<2000,"Gameplay response exceeded declared 2-second QA bound.");
            long f=0,r=0,fd=0,rd=0; foreach(var relay in relays) { f+=relay.ForwardChunks;r+=relay.ReverseChunks;fd+=relay.ForwardDelayMs;rd+=relay.ReverseDelayMs; } result["relayForwardChunks"]=f; result["relayReverseChunks"]=r; result["relayForwardMeanInjectedDelayMs"]=(double)fd/f; result["relayReverseMeanInjectedDelayMs"]=(double)rd/r;
            result["sampledPeakPrivateBytes"]=samples.MaxPrivate;result["sampledPeakWorkingSetBytes"]=samples.MaxWorking;result["sampledPeakThreads"]=samples.MaxThreads;result["sampledPeakHandles"]=samples.MaxHandles;result["resourceSampleIntervalMs"]=50;
            Check(samples.MaxPrivate-Convert.ToInt64(baseline["privateBytes"])<128L*1024*1024,"Private memory exceeded 128MiB QA growth guard.");
            result["status"]="PASS"; return Serializer().Serialize(result);
        } finally { foreach(var peer in peers) if(peer!=null) peer.Dispose(); }
    }
}
