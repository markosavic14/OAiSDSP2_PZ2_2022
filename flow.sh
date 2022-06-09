#!/bin/bash

exit 0

sudo ./waf prerequisites
./waf configure
./waf build run --app=main
