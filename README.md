# C2 BOTNET via DISCORD DLL SIDE LOADING

# Atk example

<img alt="Sequence Diagram" src="documentazione\sequence_diagram\img.png" />

# Processo del progetto

## 1. Costruire dll malevola

E' stata utilizzata la dll `WSL_32.dll` per utilizzare le socket nella dll malevola.

La dll malevola sovrascrive il comportamento di `UMPDC.dll`. è stata scelta tale dll tramite un processo iterativo, provando tutte le possibili dll che venivano cercate prima nel path `%localappdata%`. Lì è stato messa la dll malevola. Quindi verrà caricata la nostra invece che l'originale `UMPDC.dll`, localizzata in `C:\Windows\SysWOW64\umpdc.dll`.

(?) Inoltre, la DLL è stata resa più credibile possibile da antivirus e controlli, utilizzando l'utility `Invoke-DllClone`.

## 2. Injection di dll malevola su discord

Spostamento di dll malevola in cartella `C:\Users\<USER>\AppData\Local\Discord\app-<VERSION>`.
Se ci sono più versioni, lo script che sposta la dll deve copiare al dll in tutte le cartelle corrispondenti a versioni differenti di discord.

## 3. Far partire `simple_server.py`

L'attaccante fa partire il server.

## 4. Far partire `discord.exe` 

A questo punto, `discord.exe` richiamerà la dll infettata `UMPDC.dll` invece che quella originale, scatenando l'attacco e collegandosi al server C2, aspettando ordini.

## 5. Client utilizza normalmente discord mentre in background si è connesso al server hostato su `simple_server.py`

In background, il server manderà al client comandi e il client li eseguirà.

I possibili comandi sono:
1. GET HTTP REQUEST (DDoS)
    
    Parametri:
    * `website`
    * `req/min`
    * `minutes`
2. PING/PONG (per verificare che il client sia ancora vivo)
3. CLIENT_INFO (per avere informazioni del client)
4. DNS RESOLVE (può risolvere per noi un hostname)

    Parametri:
    * `url`
