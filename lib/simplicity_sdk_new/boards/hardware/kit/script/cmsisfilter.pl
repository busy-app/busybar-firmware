#!/usr/bin/perl
#
# Postprocesses stringtemplate header file to make output comply with
# Energy Micro and CMSIS code convention standards
#
# In addition corrects some errors in output, such as double
# defined entries
#
# s.olsen@energymicro.com
#
use strict;

# input array of lines, output array of lines
# remove more than one blank line in between fields
sub filter_BlankLines
{
    my @outputLines;
    my $skip;
    my $changedLines;

    @outputLines = ();

    $skip = 0;
    $changedLines = 0;

    while ($_ = shift @_)
    {
        # remove end of line character
        chomp;
        if ( $_ eq "" ) {
            $skip += 1;
            # ensure only one blank line inbetween fields
            if ( $skip == 1 ) {
                $_ = $_ . "\n";
                push @outputLines, $_;
            } else {
                $changedLines += 1;
                next;
            }
        } else {
            # if we find something else, reset blank line count
            $skip = 0;
            $_ = $_ . "\n";
            push @outputLines, $_;
        }
    }
    return @outputLines;
}


#
# Remove blank lines between multiple include statements and leading white space
#
sub filter_SpaceBetweenIncludes
{
    my @outputLines;
    my $previnc;
    @outputLines = ();

    $previnc = 0;

    while ($_ = shift @_) {
      if ( /^\s*$/ ) {
        next;
      } else {
        push @outputLines, $_;
        last;
      }
    }

    while ($_ = shift @_) {
      # include white space
      if ( /^\s*$/ ) {
        if ( $previnc == 0 ) {
          push @outputLines, $_;
        }
        next;
      }

      if (/\#include/ &&
          !(/\"\#/)) {
        $previnc = 1;
        push @outputLines, $_;
        next;
      } else {
        # add extra blank line after last include
        if ( $previnc == 1 )
        {
          push @outputLines, "\n";
        }
        $previnc = 0;
        push @outputLines, $_;
        next;
      }
    }
    return @outputLines;
}


#
# Remove white space at end of line
#
sub filter_WhiteSpaceAtEndOfLine
{
    my @outputLines;
    my $changedLines;

    @outputLines = ();

    $changedLines = 0;

    while ($_ = shift @_)
    {
        # remove end of line character
        chomp;
        if ( /\s+$/ ) {
            # whitespace at end of line, remove it
            s/\s+$//;
            $changedLines += 1;
        }
        # add to new line
        $_ = $_ . "\n";
        push @outputLines, $_;
    }
    # print "filter_WhiteSpaceAtEndOfLine: " . $changedLines . "\n";
    return @outputLines;
}

# input array of lines, output array of lines
# remove multiple typedefs
sub filter_MultipleTypedefs
{
    my @scanLines;
    my @outputLines;
    my $changedLines;
    my %typedefs;
    my $tdRemove;
    my $commentRemove = 0;

    @outputLines = ();
    @scanLines = ();
    $changedLines = 0;
    %typedefs = ();

    while ($_ = shift @_) {
        chomp;

        if ( /\} (\S+)_TypeDef\;/) {
            if ( exists $typedefs{$_} ) {
                # redefinition
                $typedefs{$_} = $typedefs{$_} + 1;
            } else {
                # first definition
                $typedefs{$_} = 1;
            }
            # enforce removal of some TypeDefs
            if ( /COREIF_TypeDef/ ) {
                $typedefs{$_} = 2;
            }
        }
        push @scanLines, $_;
    }
    # we now have a nice hash, where we can remove superflous typedefs
    # remove backwards, saving the first one
    $tdRemove = 0;
    $commentRemove = 0;
    @scanLines = reverse @scanLines;

    foreach $_ (@scanLines) {
        if ( $tdRemove == 1 ) {
            # remove until we find typedef struct at start of line
            if ( /^typedef struct/ ) {
                $commentRemove = 1;
                $tdRemove = 0;
            }
            # process next line, remove this one
            $changedLines += 1;
            next;
        }
        if ( $commentRemove == 1 ) {
            # remove until we find /** at start of line
            if ( /^\/\*\*\*\*\*\*/ ) {
                $commentRemove = 0;
            }
            # process next line, remove this one
            $changedLines += 1;
            next;
        }
        if ( /\} (\S+)_TypeDef\;/ ) {
            if ( $typedefs{$_} > 1 ) {
                $typedefs{$_} =  $typedefs{$_} - 1;
                $tdRemove = 1;
                # process next line, remove this one
                $changedLines += 1;
                next;
            }
        }
        $_ = $_ . "\n";
        push @outputLines, $_;
    }
    @outputLines = reverse @outputLines;

    # print "filter_MultipleTypedefs: " . $changedLines . "\n";

    return @outputLines;
}


#
# Align all #define statements to the left, remove documentation tags
#
sub filter_DefinesLeft
{
    my @outputLines;
    my $skip;
    my $changedLines;

    @outputLines = ();

    $skip = 0;
    $changedLines = 0;

    while ($_ = shift @_)
    {
        chomp;
        if ( /(^\s+\#define)/ ) {
            # replace leading whitespace with blank, left aligning #define
            s/(^\s+\#)/\#/;
            $changedLines += 1;
        }
        # Filter docbook tags
        if ( /\<superscript\>/ ) {
            s/\<superscript\>//;
        }
        if ( /\<\/superscript\>/ ) {
            s/\<\/superscript\>//;
        }
        if ( /\<subscript\>/ ) {
            s/\<subscript\>//;
        }
        if ( /\<\/subscript\>/ ) {
            s/\<\/subscript\>//;
        }
        push @outputLines, $_ . "\n";
    }
    # printf "filter_DefinesLeft: " . $changedLines . "\n";
    return @outputLines;
}

# input array of lines, output array of lines
# remove various fields that's not necessary in final file
sub filter_RemoveLines
{
    my @outputLines;
    my $skip;
    my $changedLines;

    @outputLines = ();
    $changedLines = 0;

    while ($_ = shift @_)
    {
        chomp;
        $skip = 0;

        if ( /PER_OFFSET/ ) {
            $changedLines += 1;
            $skip = 1;
        }

        if ( /CM([34]|0P)?_BASE/ ) {
            $changedLines += 1;
            $skip = 1;
        }

        if ( /CM([34]|0P)?_PRESENT/ ) {
            $changedLines += 1;
            $skip = 1;
        }

        if ( /CM([34]|0P)?_COUNT/ ) {
            $changedLines += 1;
            $skip = 1;
        }

        if ( /SYSTICK_(COUNT|PRESENT)/ ) {
            $changedLines += 1;
            $skip = 1;
        }

        if ( /DEC_PRESENT/ ) {
            $changedLines += 1;
            $skip = 1;
        }

        if ( /DEC_COUNT/ ) {
            $changedLines += 1;
            $skip = 1;
        }

        # will also be handled by lines below
        if ( /COREIF_TypeDef/ ) {
            $changedLines += 1;
            $skip = 1;
        }

        if ( $skip == 0 ) {
            push @outputLines, $_ . "\n";
        }
    }
    # printf "filter_RemoveLines: " . $changedLines . "\n";
    return @outputLines;
}

# input array of lines, output array of lines
# remove various fields that's not necessary in final file
sub filter_RemoveCMxNVICAndSYSTICK
{
    my @outputLines;
    my $skip;
    my $tmp;

    @outputLines = ();

    $skip = 0;
    while ($_ = shift @_)
    {
        chomp;

        if ( /^\/\*\*/ ) {
            # This line is a comment. Add it to the next line
            $tmp = $_;
            $_ = shift @_;
            chomp;
            $_ = $tmp . "\n" . $_;
        }
        # remove the entire bit defs for CM3/4/M0P_NVIC/SYSTICK
        if ( /addtogroup/ || /defgroup/ ) {
            if ( /_CM_|_CM0P_|_SYSTICK_|_SYSTICK$|_CM$|_CM0P$/ ) {
                $skip = 1;
            }
        }
        if ( $skip == 0 ) {
            push @outputLines, $_ . "\n";
        }
        if ( $skip == 1 ) {
            if ( /\@\}/ ) {
                $skip = 2;
            }
        }
        if ( $skip == 2 ) {
            if ( /\*\// ) {
                $skip = 0;
            }
        }
    }
    return @outputLines;
}


# input array of lines, output array of lines
# remove various fields that's not necessary in final file
sub filter_RemoveDEC
{
    my @outputLines;
    my $skip;
    my $tmp;

    @outputLines = ();

    $skip = 0;
    while ($_ = shift @_)
    {
        chomp;

        if ( /^\/\*\*/ ) {
            # This line is a comment. Add it to the next line
            $tmp = $_;
            $_ = shift @_;
            chomp;
            $_ = $tmp . "\n" . $_;
        }
        # remove the entire bit defs for DEC (AAP)
        if ( /addtogroup.*_DEC\W*$/ ) {
            $skip = 1;
        }
        if ( $skip == 0 ) {
            push @outputLines, $_ . "\n";
        }
        if ( $skip == 1 ) {
            if ( /\@\}/ ) {
                $skip = 2;
            }
        }
        if ( $skip == 2 ) {
            if ( /\*\// ) {
                $skip = 0;
            }
        }
    }
    return @outputLines;
}

sub filter_FixTimerWidth {
    # This is placed here because fixing this in the jsonparser would
    # require significant restructuring of the data model and the TIMER
    # is the only expected case of this left on the 90nm platform.
    my @inputLines = @_;
    my @outputLines;
    foreach my $inputLine (@inputLines) {
        $inputLine =~ s/(_TIMER_CNT_MASK +)0xFFFFFFFFUL/${1}0x0000FFFFUL/;
        $inputLine =~ s/(_TIMER_CNT_CNT_MASK +)0xFFFFFFFFUL/${1}0xFFFFUL/;
        $inputLine =~ s/(_TIMER_TOPB?_MASK +)0xFFFFFFFFUL/${1}0x0000FFFFUL/;
        $inputLine =~ s/(_TIMER_TOPB?_TOPB?_MASK +)0xFFFFFFFFUL/${1}0xFFFFUL/;
        $inputLine =~ s/(_TIMER_CC_CCV[PB]?_MASK +)0xFFFFFFFFUL/${1}0x0000FFFFUL/;
        $inputLine =~ s/(_TIMER_CC_CCV[PB]?_CCV[PB]?_MASK +)0xFFFFFFFFUL/${1}0xFFFFUL/;
        push(@outputLines, $inputLine);
    }
    return @outputLines;
}

# input array of lines, output array of lines
# if previous item was a #define, space and a new #define
# remove space
sub filter_CollapseDefines
{
    my @outputLines;
    my $skip;
    my $changedLines;
    my $sequence;

    @outputLines = ();
    $changedLines = 0;
    # 3 stage sequence, first a #define, then a blank line, then a #define
    $sequence = 0;

    while ($_ = shift @_)
    {
        chomp;
        if ( $sequence == 0 ) {
            if ( /^\#define/ ) {
                $sequence = 1;
                push @outputLines, $_ . "\n";
                next;
            }
            push @outputLines, $_ . "\n";
            next;
        }
        if ( $sequence == 1 ) {
            if ( $_ eq "" ) {
                # do not output blank line yet
                $sequence = 2;
                next;
            }
            if ( /^\#define/ ) {
                $sequence = 1;
                push @outputLines, $_ . "\n";
                next;
            }
            $sequence = 0;
            push @outputLines, $_ . "\n";
            next;

        }
        if ( $sequence == 2 ) {
            if ( /^\#define/ ) {
                # do not output previous blank line
                $changedLines += 1;
                $sequence = 1;
                push @outputLines, $_ . "\n";
                next;
            }
            if ( $_ eq "" ) {
                # yet another blank line - ignore it
                $changedLines += 1;
                next;
            } else {
                # output prevous blank line
                push @outputLines, "\n" ;
                # output current content
                push @outputLines, $_ . "\n";
                $sequence = 0;
                next;
            }
        }
    }
    # print "filter_CollapseDefines: " . $changedLines . "\n";
    return @outputLines;
}

sub filter_conditionals
{
    my $filePath = shift;
    my @outputLines;
    my $partName = "";

    @outputLines = ();
    if ($filePath =~ m|.*/([^/]+)\.h|) {
        $partName = uc $1;
    }
    my $deviceKeyFile = "../autogen/devicekeys/${partName}.dita";

    while ($_ = shift @_)
    {
        chomp;
        if(m|<([^ ]+) .*condition="([^>^"^ ]+)".*>(.*)</\1>|) { # If line contains xml tag with a contition
            my $tag = $1;
            my $condition = $2;
            my $content = $3;

            if (-e $deviceKeyFile) {
                my $res = `../toolchain/dita_condition_filter.py --evalTag ${deviceKeyFile} ${condition}`;
                unless ($res =~ m/^[0-9]+$/) {
                    print "\nERROR: condition evatuation returned non-numeric value\n";
                }
                my $conditionalText = ($res != 0) ? $content : "";
                s|<${tag}[^>]*>.*</${tag}>|${conditionalText}|; # Replace tag
            }
        }
        push @outputLines, $_ . "\n";
    }
    return @outputLines;
}

sub writeFile
{
    my $filename;

    $filename = shift @_;

    # print "New line count:" . $# . "\n";

    open (DFILE, ">".$filename ) || die "Couldn't write to file " . $filename . "\n";
    while( $_ = shift @_ ) {
        print DFILE $_;
    }
    close DFILE;
}

sub changeFile
{
    my $filename;
    my @filedata;
    my $line;

    $filename = $_[0];
    @filedata = ();

    open (DFILE, $filename ) || die "Couldn't read file " . $filename . "\n";
    @filedata = <DFILE>;
    close (DFILE);

    # apply filters
    print "Run filters on file " . $filename . "...(" . $#filedata . " to ";

    # note! order is important. removelines will lead to more blanklines
    @filedata = filter_WhiteSpaceAtEndOfLine @filedata;
    @filedata = filter_MultipleTypedefs @filedata;
    @filedata = filter_RemoveLines @filedata;
    @filedata = filter_RemoveCMxNVICAndSYSTICK @filedata;
    @filedata = filter_RemoveDEC @filedata;
    @filedata = filter_DefinesLeft @filedata;
    @filedata = filter_CollapseDefines @filedata;
    @filedata = filter_BlankLines @filedata;
    @filedata = filter_SpaceBetweenIncludes @filedata;
    @filedata = filter_FixTimerWidth @filedata;
    @filedata = filter_conditionals($filename, @filedata);
    print $#filedata . " lines)\n";

    # write back file
    writeFile $filename, @filedata;
}


while ($ARGV[0])
{
    changeFile $ARGV[0];
    shift @ARGV;
}
