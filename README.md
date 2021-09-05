
1.Build command
===================

1). Open makefile and modify SRCDIR to current path 

e.g SRCDIR = /home/asit/Desktop/TS-Extractor

2). make

2.Commands to run :
==================

ffmpeg -re -i ed24p_00.ts -c:v copy -c:a copy -f mpegts udp://127.0.0.1:5000?pkt_size=188

sudo ./TS-Extractor

3.Command to run stream repeatedly :
==================================
ffmpeg -re -stream_loop -1 -i ed24p_00.ts -c:v copy -c:a copy -f mpegts udp://127.0.0.1:5000?pkt_size=188


4.Stream test commands :
=====================
ffplay udp://127.0.0.1:5000


5.Show TS packets info :
======================
ffprobe -v quiet  -show_packets -count_packets ed24p_00.ts >> text.txt

ffprobe -v quiet -print_format json -show_format -show_streams -show_packets ed24p_00.ts >> info.txt
