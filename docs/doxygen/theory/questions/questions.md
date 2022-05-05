# DUBBI RISOLTI

## Come suddividere messaggi non divisibili per 4

DOMANDA:

Siccome l'ultima parte del messaggio e' l'unica che puo' essere piu' corta per specifica... Cosa bisogna fare in casi in cui non e' possibile garantire questo vincolo?

Esempio: 2 caratteri possono essere divisi in:
- caratteri per parte: 1 1 0 0
- oppure: 2 0 0 0

Lo stesso problema si pone per 1, 2, 5, 6, 9, 10, ... caratteri.

Non posso garantire come ad esempio nel caso di 3 caratteri che sono l'ultimo numero sia inferiore:

Esempio di suddivisione di 3 caratteri: 1 1 1 0. L'ultimo, come per specifica, e' l'unico di dimensione inferiore

RISPOSTA:

Va bene l'algoritmo attualmente utilizzato: il contenuto di un file viene diviso in parti uguali e intere tra i canali di comunicazione mentre il resto viene distribuito un pezzo per canale di comunicazione.

Esempi:

|Numero caratteri|Caratteri per FIFO 1|Caratteri per FIFO 2|Caratteri per msgqueue|Caratteri per memoria condivisa|
|----------------|--------------------|--------------------|----------------------|-------------------------------|
|0               |0                   |0                   |0                     |0                              |
|1               |1                   |0                   |0                     |0                              |
|2               |1                   |1                   |0                     |0                              |
|3               |1                   |1                   |1                     |0                              |
|4               |1                   |1                   |1                     |1                              |
|5               |2                   |1                   |1                     |1                              |
|6               |2                   |2                   |1                     |1                              |
|7               |2                   |2                   |2                     |1                              |
|8               |2                   |2                   |2                     |2                              |
|9               |3                   |2                   |2                     |2                              |
|10              |3                   |3                   |2                     |2                              |
|...             |...                 |...                 |...                   |...                            |

## dimensione massima percorsi file

DOMANDA:

I percorsi dei file hanno dimensione massima?

RISPOSTA:

Non sappiamo la lunghezza massima dei percorsi dei file ma sappiamo che sono molto corti.

I test verranno eseguiti nella cartella ```/home``` oppure ```/runner``` e ci saranno al massimo 3 o 4 sottocartelle: probabilmente non si superera' il centinaio di caratteri.
> Attualmente vengono supportati percorsi di lunghezza massima di 255 caratteri

## Serve la documentazione?

DOMANDA:

La specifica non richiede la documentazione. E' richiesta?

RISPOSTA:

Non e' mai stata richiesta, quindi in teoria no.

## Caratteri non ASCII

DOMANDA:

I caratteri nei file di testo in input ai client sono ASCII? Sono tutti da 1 byte? Bisogna gestire lettere accentate, ...?

RISPOSTA:

I caratteri che verranno letti sono tutti da un byte e di conseguenza non c'e' da preoccuparsi di caratteri non ASCII.
