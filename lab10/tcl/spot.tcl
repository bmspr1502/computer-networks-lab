#create a simulator object
set ns [new Simulator]

#create a trace file and animation file
set tracefile [open LAN.tr w]
$ns trace-all $tracefile

set namfile [open LAN.nam w]
$ns namtrace-all $namfile

#creating 5 nodes
set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]
set n4 [$ns node]
set n5 [$ns node]

$ns duplex-link $n0 $n1 5Mb 2ms DropTail
$ns duplex-link $n2 $n1 10Mb 4ms DropTail
$ns duplex-link $n1 $n4 3Mb 10ms DropTail
$ns duplex-link $n4 $n3 100Mb 2ms DropTail
$ns duplex-link $n4 $n5 4Mb 10ms DropTail

#creation of UDP agents from n0 to n3
set UDP [new Agent/UDP]
set null [new Agent/Null]

$ns attach-agent $n0 $UDP
$ns attach-agent $n3 $null
$ns connect $UDP $null

#creation of TCP agent from n2 to n5
set TCP [new Agent/TCP]
set sink [new Agent/TCPSink]

$ns attach-agent $n2 $TCP
$ns attach-agent $n5 $sink
$ns connect $TCP $sink

#creation of application cbr(UDP) and ftp(TCP) in local LAN
set cbr [new Application/Traffic/CBR]

$cbr attach-agent $UDP

set ftp [new Application/FTP]
$ftp attach-agent $TCP

#start the traffic
$ns at 1.0 "$cbr start" 
$ns at 2.0 "$ftp start"
$ns at 10.0 "finish"

proc finish {} {

global ns tracefile namfile
$ns flush-trace

close $tracefile
close $namfile

exit 0
}

puts "LAN Network with 5 nodes" 
puts "CBR connection UDP packets flow from node0 to node3" 
puts "FTP connection TCP packets flow from node2 to node5" 
$ns run