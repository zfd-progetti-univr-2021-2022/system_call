/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

/// Buffer usato da getcwd()
#define BUFFER_SZ 255

// -- Macro suddivisione messaggi

// > 4 KB -> 4096 byte -> 1024 caratteri da 1 byte ciascuno per le 4 parti dei messaggi

/// dimensione massima del messaggio/file da inviare
#define MSG_MAX_SZ 4096
/// numero parti in cui suddividere il messaggio
#define MSG_PARTS_NUM 4
/// dimensione massima di una porzione di messaggio
#define MSG_BUFFER_SZ (MSG_MAX_SZ / MSG_PARTS_NUM)
