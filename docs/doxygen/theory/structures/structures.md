# Teoria: strutture

## Struttura dei permessi delle IPC

Tutte le IPC permettono di ottenere o impostare una struttura che contiene il campo di tipo ```struct ipc_perm``` che contiene i permessi e il proprietario della IPC.

```c
struct ipc_perm {
    key_t __key; /* chiave IPC */
    uid_t uid; /* ID utente proprietario */
    gid_t gid; /* ID gruppo del proprietario */
    uid_t cuid; /* ID dell'utente creatore */
    gid_t cgid; /* ID del gruppo del creatore */
    unsigned short mode; /* Permessi */
    unsigned short __seq; /* Numero di sequenza */
};
```

Dove:
* ```__key``` puo' essere utile per leggere la chiave di IPC_PRIVATE
* ```ciod``` e ```cgid``` sono immutabili
* Impostare i permessi di esecuzione in ```mode``` non serve a niente: contano soltanto i permessi di lettura e di scrittura
