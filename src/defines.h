/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#define BUFFER_SZ 255

// 4 KB -> 4096 byte -> 1024 caratteri da 1 byte ciascuno per le 4 parti dei messaggi
#define MSG_MAX_SZ 4096
#define MSG_PARTS_NUM 4
#define MSG_BUFFER_SZ (MSG_MAX_SZ / MSG_PARTS_NUM)
