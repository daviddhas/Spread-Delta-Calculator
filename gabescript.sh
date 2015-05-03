#!/bin/bash 

## Fixes the persistant permissions error Gabe has been having

chown -R demo:demo *
chown -R demo:demo .git
chown -R demo:demo .gitignore
