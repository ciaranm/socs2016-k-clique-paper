#!/bin/bash
# vim: set sw=4 sts=4 et :

LIST=$1
FORMAT=$2

RESULTS_ccon=../experiments/results/sequential
RESULTS_cconlgd=../experiments/results/sequential-lgd
RESULTS_about=../experiments/results/about

cat $1 | while read filename name ; do

    for power in 2 3 4 ; do
        if [[ $power == 2 ]] ; then
            echo $name
        elif [[ $power == 3 ]] ; then
            # |V|
            echo '\hspace*{0.5em}\color{gray}$|V|{=}'$(../code/about_graph --format $FORMAT ../instances/$filename | cut -d' ' -f1)'$'
        elif [[ $power == 4 ]] ; then
            # |E|
            echo '\hspace*{0.5em}\color{gray}$|E|{=}'$(../code/about_graph --format $FORMAT ../instances/$filename | cut -d' ' -f2)'$'
        fi

        echo "& $power"

        # density
        echo "&"
        read v e x <<<$(<${RESULTS_about}-${power}/$name.out)
        ruby -e "printf('%.2f', (2.0 * $e) / ($v * ($v - 1.0)))"

        # omega
        echo "&"
        FILE=${RESULTS_cconlgd}-${power}/$name.out
        if grep -q aborted $FILE ; then
            echo '${\ge}'$(head -n1 $FILE | cut -d' ' -f1)'$'
        else
            head -n1 $FILE | cut -d' ' -f1
        fi

        for method in "ccon" "cconlgd" ; do
            resultsdir=RESULTS_$method
            FILE=${!resultsdir}-${power}/$name.out

            if [[ -f $FILE ]] ; then
                if grep -q aborted $FILE ; then
                    rc='\color{gray}'
                else
                    rc=
                fi
            fi

            # nodes
            echo "&"
            echo $rc
            n=$(head -n1 $FILE | cut -d' ' -f2)
            if [[ $n -ge 1000000 ]] ; then
                ruby -e "printf(\"\\\\num{%.1e}\", $n)"
            else
                echo $n
            fi

            # time
            echo "&"
            echo $rc
            t=$(tail -n1 $FILE | cut -d' ' -f1 )
            if [[ $t -ge 86400000 ]] ; then
                echo '$>$ 1 day'
            else
                [[ -s $FILE ]] && ruby -e "printf(\"%.1f \", $t / 1000.0)"
            fi
        done

        echo
        if [[ $power == 4 ]] ; then
            echo "\\\\[0.2cm]"
        else
            echo "\\\\*"
        fi
    done
done | sed -e '$d'

echo "\\\\"

