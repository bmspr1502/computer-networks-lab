# new simulator
set ns [new Simulator]

# different colors for different dataframes
$ns color 1 Green
$ns color 2 Brown

set tracefile [open pran.tr w]
$ns trace-all $tracefile

set namfile [open pran.nam w]
$ns namtrace-all $namfile

# finish procedure
proc finish {} {
    global ns nf
    $ns flush-trace
    exec nam pran.nam &
    exit 0
}


# 4 main nodes
set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]

# links between nodes
$ns duplex-link $n0 $n2 2Mb 10ms DropTail
$ns duplex-link $n1 $n2 2Mb 10ms DropTail
$ns duplex-link $n2 $n3 1.7Mb 20ms DropTail

# set queue size of link (n2 - n3) to 10
$ns queue-limit $n2 $n3 15

# node positions for nam
$ns duplex-link-op $n0 $n2 orient right-down
$ns duplex-link-op $n1 $n2 orient right-up
$ns duplex-link-op $n2 $n3 orient right

# monitor queue for (n2 - n3)
$ns duplex-link-op $n2 $n3 queuePos 0.5

# TCP connection
set tcp [new Agent/TCP]
$tcp set class_ 2
$ns attach-agent $n0 $tcp

set sink [new Agent/TCPSink]
$ns attach-agent $n3 $sink
$ns connect $tcp $sink
$tcp set fid_ 1

# color code 1
# FTP over TCP
set ftp [new Application/FTP]
$ftp attach-agent $tcp
$ftp set type_ FTP

# UDP connection
set udp [new Agent/UDP]
$ns attach-agent $n1 $udp

set null [new Agent/Null]
$ns attach-agent $n3 $null
$ns connect $udp $null

$udp set fid_ 2

# color code 2
# CBR over UDP
set cbr [new Application/Traffic/CBR]

$cbr attach-agent $udp
$cbr set type_ CBR
$cbr set packet_size_ 1000
$cbr set rate_ 1mb
$cbr set random_ false

# events and scheduling
$ns at 0.1 "$cbr start"
$ns at 1.0 "$ftp start"
$ns at 4.0 "$cbr stop"
$ns at 4.5 "$cbr stop"

# finish
$ns at 5.6 "finish"

# print CBR packet size and interval
puts "CBR packet size : [$cbr set packet_size_]"
puts "CBR interval : [$cbr set interval_]"

$ns run
