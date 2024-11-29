#!/usr/bin/env bash

perl ./cinclude2dot.pl --quotetypes quote --src src > graph.dot
sccmap -v graph.dot
dot -T svg graph.dot > graph.svg
