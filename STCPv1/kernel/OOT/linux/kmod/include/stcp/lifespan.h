#pragma once
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/net.h>
#include <linux/atomic.h>

/*
 * Kuinka monta STCP-kontekstia / socketia on elossa.
 * Tämä ei sinällään estä moduulin unloadia, mutta antaa
 * debug-infon jos jokin jää roikkumaan.
 */

void stcp_exported_rust_sockets_alive_get(void);
void stcp_exported_rust_sockets_alive_put(void);
int  stcp_exported_rust_ctx_alive_count(void);

int stcp_end_of_life_for_sk(void *skvp, int err);
