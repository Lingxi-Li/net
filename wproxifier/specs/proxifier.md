## Connection Establishment

```mermaid
sequenceDiagram
autonumber
client->>proxifier: SYN
proxifier->>proxy: SYN
proxy-->>proxifier: SYN ACK
proxifier-->>proxy: ACK
proxifier-->>proxy: CONNECT
proxy-->>proxifier: ACK
proxy-->>proxifier: CONNECTED
proxifier->>client: SYN ACK
client->>proxifier: ACK
proxifier->>proxy: ACK
```
