/// @file strings.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione delle STRINGHE.

#pragma once

#include <stdbool.h>

/**
 * @brief Concatena la cartella directory al percorso path.
 *
 * @param path Percorso root a cui aggiungere directory
 * @param directory Nome cartella da aggiungere al path
 * @return size_t Dimensione caratteri di directory
 */
size_t append2Path(char *path, char *directory);

/**
 * @brief Restituisce vero se la stringa a inizia con la sotto stringa b
 *
 * @param a Stringa che potrebbe iniziare con b
 * @param b Stringa che potrebbe essere contenuta all'inizio di a
 * @return true La stringa a inizia con la sotto stringa b
 * @return false La stringa a NON inizia con la sotto stringa b
 */
bool StartsWith(const char *a, const char *b);
