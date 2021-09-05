
1.Build command
===================

* Open makefile and modify SRCDIR to current path  (  e.g SRCDIR = /home/asit/Desktop/TS-Extractor  )

* make

2.Commands to run :
==================

* install ffmpeg ( sudo apt install ffmpeg )

* ffmpeg -re -i sample.ts -c:v copy -c:a copy -f mpegts udp://127.0.0.1:5000?pkt_size=188

* sudo ./TS-Extractor

3.Command to run stream repeatedly :
==================================
* ffmpeg -re -stream_loop -1 -i sample.ts -c:v copy -c:a copy -f mpegts udp://127.0.0.1:5000?pkt_size=188


4.Stream test commands :
=====================
* ffplay udp://127.0.0.1:5000


5.Show TS packets info :
======================
* ffprobe -v quiet  -show_packets -count_packets sample.ts >> text.txt

* ffprobe -v quiet -print_format json -show_format -show_streams -show_packets sample.ts >> info.txt
