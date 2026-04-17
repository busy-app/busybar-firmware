#!/usr/bin/perl -w
use warnings;
use strict;

open INP_FILE, "< ".$ARGV[0];
open OUT_FILE, "> ".$ARGV[1];


our $ipmu_base_addr ;
our $pmu_base_addr ;
our $start = 0 ;
our @info ;
our $result ;
our @addr_final ;
our @data_final ;
our $i ;
our $k ;
our @reg ;
our @reg1 ;


@info = "" ;
$ipmu_base_addr = "2405A000" ;
$pmu_base_addr = "24058000" ;
$i = 0;
$k = 0;
@reg = "" ;
@reg1 = "" ;


#print "PATH_NUMBER\t Startpoint\t Endpoint\t Path Group\t Slack\t No of levels \n" ;
FILE: while (<INP_FILE>)
{
	chomp; #remove \n
	if ($start == 1)
		{
	    @info =split(/,/,$_);
			if ($info[3] == "1" || $info[3] == "2" ) 
			{
				$i++ ;
				if ($info[3] == 1)
				{
					$addr_final[$i] = hex($ipmu_base_addr) + ((hex($info[4])*4) ) ;
				} elsif ($info[3] == 2) {
					$addr_final[$i] = hex($pmu_base_addr) + ((hex($info[4])*4) ) ;
				}
					$data_final[$i] = $info[5]*(2 ** 27)+$info[6]*(2 ** 22)+hex($info[7]); 
					$addr_final[$i] = sprintf("0x%X", $addr_final[$i]);
					$data_final[$i] = sprintf("0x%X", $data_final[$i]);
					#print "$addr_final[$i] : $data_final[$i] \n" ;
					$reg[$i] = $info[1] ;
					$reg1[$i] = $info[2] ;
			}
			if ($info[3] == "")	
			{
				print OUT_FILE "uint32_t $reg[1]\[\] _COMMON_IPMU_RAM_ \/\/$reg1[1] \nuint32_t $reg[1]\[\] = {\n\t$i,\n" ;
				for ($k=0 ;  $k<$i ; $k++)
				{
					print OUT_FILE "$addr_final[$k+1],$data_final[$k+1],\n" ;
				}
				print OUT_FILE "};\n" ;
				$i=0;
			}
		}                      	
	if (/TYPE.*MSB.*LS.*/) 	
		{                      	
		$start = 1;            	
		}                      	
}                              	
                               	
print OUT_FILE "uint32_t $reg[1]\[\] = {\n\t$i,\n" ;
for ($k=0 ;  $k<$i ; $k++)
{
	print OUT_FILE "$addr_final[$k+1],$data_final[$k+1],\n" ;
}
print OUT_FILE "};\n" ;
$i=0;
                               	
