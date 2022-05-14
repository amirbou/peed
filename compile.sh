#!/bin/bash
exec gcc src/test.c src/peed.c -I./inc -g -D_DEBUG src/proc.c src/net.c
