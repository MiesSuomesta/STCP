#pragma once
#include <net/sock.h>
#include <linux/printk.h>
#include <linux/socket.h>
#include <linux/list.h>
#include <linux/version.h>
#include <linux/printk.h>
#include <linux/socket.h>  
#include <linux/kernel.h>  // container_of
#include <net/sock.h>      // struct sock
#include <linux/net.h>     // struct socket
#include <net/inet_connection_sock.h>

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

#include "lifecycle.h"
#include "structures.h"
#include "sock_glue.h"
#include "operations.h"
#include "lifecycle.h" // Turha ?
#include "helpers.h"
#include "rust_hooks.h"
#include "state.h"
#include "the_context.h"

extern int stcp_safe_mode;
