#!/usr/bin/env bash

# Runs performance tests.
# Here we run Edlib with different sizes of queries, with special focus on short queries,
# since that is where Edlib needs to improve.

EDLIB=$(dirname $(pwd))/build/bin/edlib-aligner
LVALG2=~/Git/Zavrsni_Rad/build/bin/main

TEST_DATA=.

########################## TEST RUNNERS ########################

F_YELLOW='\e[93m'
F_NONE='\e[0m'
F_UNDERLINE='\e[4m'
F_NO_UNDERLINE='\e[24m'
F_BOLD='\e[1m'
F_NO_BOLD='\e[21m'

function echo_underlined {
    local input=$1
    echo -e "${F_UNDERLINE}${input}${F_NO_UNDERLINE}"
}

function echo_bolded {
    local input=$1
    echo -e "${F_BOLD}${input}${F_NO_BOLD}"
}

function edlib {
    mode=$1
    query=$2
    target=$3
    num_tests=$4
    k=$5
    r=${6-1}  # Number of repetitions, default is 1.

    time_sum=0
    for i in $(seq $num_tests); do
        sleep 0.25
        output=$($EDLIB -m $mode -k $k -r $r $query $target)
        time=$(echo "$output" | grep "Cpu time of searching" | cut -d " " -f5)
        score=$(echo "$output" | grep "#0:" | cut -d " " -f2)
        time_sum=$(python -c "print($time_sum + $time)")
        #echo ">" "#"$i $score $time
    done
    avg_time=$(python -c "print($time_sum / $num_tests)")
    echo -e "    => Edlib, $r repetition(s): time=${avg_time}s, score=$score"
}

function landauVishkin {
    mode=$1
    query=$2
    target=$3
    num_tests=$4
    r=$5
        
    time_sum=0
    for i in $(seq $num_tests); do
        sleep 0.25
        output=$($LVALG2 $mode $query $target $r)
        time=$(echo "$output" | grep "Cpu time of searching" | cut -d " " -f5)
        score=$(echo "$output" | grep "#0:" | cut -d " " -f2)
        time_sum=$(python -c "print($time_sum + $time)")
        #echo ">" "#"$i $score $time
    done
    avg_time=$(python -c "print($time_sum / $num_tests)")
    echo -e "    => LandauVishkin, $r repetition(s): time=${avg_time}s, score=$score"
}

function landauVishkinPath {
    mode=$1
    query=$2
    target=$3
    num_tests=$4
    r=$5
    p=$6
        
    time_sum=0
    for i in $(seq $num_tests); do
        sleep 0.25
        output=$($LVALG2 $mode $query $target $r $p)
        time=$(echo "$output" | grep "Cpu time of searching" | cut -d " " -f5)
        score=$(echo "$output" | grep "#0:" | cut -d " " -f2)
        time_sum=$(python -c "print($time_sum + $time)")
        #echo ">" "#"$i $score $time
    done
    avg_time=$(python -c "print($time_sum / $num_tests)")
    echo -e "    => LandauVishkin, $r repetition(s): time=${avg_time}s, score=$score"
}

function edlib_path {
    mode=$1
    query=$2
    target=$3
    num_tests=$4
    r=${5-1}  # Number of repetitions, default is 1.

    time_sum=0
    for i in $(seq $num_tests); do
        sleep 0.1
        output=$($EDLIB -m $mode -r $r -p -s $query $target)
        time=$(echo "$output" | grep "Cpu time of searching" | cut -d " " -f5)
        time_sum=$(python -c "print($time_sum + $time)")
        #echo ">" "#"$i $time
    done
    avg_time=$(python -c "print($time_sum / $num_tests)")
    echo -e "    => Edlib (path), $r repetition(s): time=${avg_time}s, score=$score"
}


############################ TESTS #############################



# ---------------------- Short reads ----------------------- #
# HW (infix).
echo_bolded "\nHW, short query (<= 500bp) and long target."
target=$TEST_DATA/E_coli_DH1/e_coli_DH1.fasta
for query_length in 50 100 250 500; do
    echo_underlined "Query length: $query_length"
    for query in $(ls $TEST_DATA/E_coli_DH1/mason_illumina_reads/${query_length}bp/*.fasta); do
        if [ $query == $TEST_DATA/E_coli_DH1/mason_illumina_reads/${query_length}bp/e_coli_DH1_illumina_1x${query_length}.fasta ]; then
        echo "  Query: $query"
        edlib      HW $query $target 3 -1
        edlib_path HW $query $target 3
        landauVishkin HW $query $target 1 1
        fi
        if [ $query == $TEST_DATA/E_coli_DH1/mason_illumina_reads/${query_length}bp/mutated_90_perc.fasta ]; then
        echo "  Query: $query"
        edlib      HW $query $target 3 -1
        edlib_path HW $query $target 3
        landauVishkin HW $query $target 1 1
        fi
        if [ $query == $TEST_DATA/E_coli_DH1/mason_illumina_reads/${query_length}bp/mutated_94_perc.fasta ]; then
        echo "  Query: $query"
        edlib      HW $query $target 3 -1
        edlib_path HW $query $target 3
        landauVishkin HW $query $target 1 1
        fi
        if [ $query == $TEST_DATA/E_coli_DH1/mason_illumina_reads/${query_length}bp/mutated_97_perc.fasta ]; then
        echo "  Query: $query"
        edlib      HW $query $target 3 -1
        edlib_path HW $query $target 3
        landauVishkin HW $query $target 1 1
        fi
    done
done
exit 0

# ----------------------- Long reads ----------------------- #

# HW (infix).
echo_bolded -e "\nHW, long query and long target."
target=$TEST_DATA/E_coli_DH1/e_coli_DH1.fasta
for query in $(ls $TEST_DATA/E_coli_DH1/mason_illumina_reads/10kbp/*.fasta); do
    echo_underlined "Query: $query"
    edlib      HW $query $target 3 -1
    edlib_path HW $query $target 3
done

# SHW (prefix).
echo_bolded -e "\nSHW, long query and long target."
target=$TEST_DATA/E_coli_DH1/e_coli_DH1.fasta
for query in $(ls $TEST_DATA/E_coli_DH1/prefixes/10kbp/mutated_*_perc.fasta); do
    echo_underlined "Query: $query"
    edlib      SHW $query $target 3 -1
    edlib_path SHW $query $target 3
done

# NW (global).
echo_bolded "\nNW, big query and target."
target=$TEST_DATA/Chromosome_2890043_3890042_0/Chromosome_2890043_3890042_0.fasta
for query in $(ls $TEST_DATA/Chromosome_2890043_3890042_0/mutated_*_perc.fasta); do
    echo_underlined "Query: $query"
    edlib      NW $query $target 3 -1
    edlib_path NW $query $target 3
done
