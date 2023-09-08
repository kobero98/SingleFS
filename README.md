# SingleFS
Progetto per il corso di Sistemi Operativi Avanzati della facoltà di Ingegneria Informatica Magistrale dell'Università di Roma - Tor Vergata nell'anno accademico 22/23
## Autore
- Matteo Federico 0321569
## Scopo
Il progetto prevede la creazione di un block device driver e l'implementazione delle operazioni di open, read, release.<br>
Inoltre bisognava implementare tre system call per poter utilizzare il device senza dover utilizzare le A.P.I. del Virtual File System, in particolare, le tre systemcall da implementare sono:
- int put (char * buffer,int size): per inserire un nuovo blocco di dati
- int get(int index,char *buffer,int size): per ottenere le informazioni di un blocco specificato come parametro
- int invalide(int offset): per eliminare il blocco indicato come parametro
## Implementazione
Ogni blocco ha dimensione fissata di 4096 Byte, la struttura dei blocchi varia a seconda del blocco che si deve utilizzare, la struttura prevede:
- un Super Blocco: posto nella prima posizione del device, contiene il magi number del device, la versione, il numero di blocchi del device,il numero M di blochi di metadati, e ulteriori informazioni per poter montare correttamente il device.
- M blocchi di metadati: che permettono di tenere traccia delle operazioni che sono staate effettuate sul dispositivo e l'ordine delle quali sono state eseguite.
- un blocco riservato per l'unico inode
- N blocchi di dati: ogni blocco di dati é strutturato con un campo indicande la dimensione dei dati e il campo dei dati.
## Pre-requisiti
Il modulo per essere compilato ed eseguito necessità di:
- *gcc*
- *Make*
Il sistema é stato provato sulle seguenti versioni del kernel: {6.4.12, 6.4.8, 6.2.0,5.15.0}

## Compilazione e Montaggio
Nella cartela principale del progetto eseguire il comando
```
make
```
verranno compilati e montanti i moduli (richiede la password di sudo)<br>
in seguito eseguire i comandi:
```
make create-fs
make mount-fs
```
Il primo comando permetterà di compilare il programma che si occuperà dell'inizializzazione di un file-system (con già 8 blocchi pre-caricati come esempio), mentre il secondo monterà il file system creato <br>
N.B. non é necessario creare ogni volta il file system, il sistema può partire da un file system già esistente e caricato.<br>
in seguito si può eseguire il comando
```
make create-user
```
per compilare nella cartella user alcuni programmi di esempio che eseguono le systemcall o alcuni test per vedere la corretta funzionalità del sistema<br>

