#!/bin/bash -e

git add ../data/gui/core_lang*

git commit -m "Sync translations."

git push origin master
