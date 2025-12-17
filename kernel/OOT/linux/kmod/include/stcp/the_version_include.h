#pragma once
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/net.h>
#include <linux/errno.h>

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif


#ifndef STCP_BUILD_DATE
#define STCP_BUILD_DATE "unknown"
#endif

#ifndef STCP_GIT_SHA
#define STCP_GIT_SHA "unknown"
#endif

#ifndef STCP_VERSION
#define STCP_VERSION "0.0.1-beta"
#endif 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lauri Jakku / Paxsudos IT <lauri.jakku@paxsudos.fi>");
MODULE_DESCRIPTION("STCP: SecureTCP, drop in replacement for TCP socket");

MODULE_VERSION(STCP_VERSION "-" STCP_GIT_SHA "-" STCP_BUILD_DATE);
MODULE_ALIAS_NETPROTO(IPPROTO_STCP);
MODULE_SOFTDEP("pre: tcp");
MODULE_INFO(company, "Paxsudos IT");
MODULE_INFO(product, "STCP Module for linux");
MODULE_INFO(support, "info@paxsudos.fi");


static void stcp_kernel_banner(void)
{
    pr_emerg(".----<[STCP by Paxsudos IT]>------------------------------------------------------------>\n");
    pr_emerg("|  ✅ STCP Initialised (Version %s), Protocol number %d\n", STCP_VERSION, IPPROTO_STCP);
    pr_emerg("|  🕓 Build at %s (%s)\n", STCP_BUILD_DATE, STCP_GIT_SHA);
    pr_emerg("'----------------------------------------------------------------------'\n");
}

