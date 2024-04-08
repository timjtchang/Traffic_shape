#!/bin/tcsh -f

echo "warmup2 my test"

                /bin/rm -f f?.out f?.ana

                make

                # echo "===> test_1, wait 5 seconds then press <Cntrl+C>"
                # ./warmup2 -t w2data/f1.txt -B 2 > f1.out
                # ./analyze-trace.txt -t w2data/f1.txt -B 2 f1.out > f1.ana


                #./warmup2 -lambda 0.99 -mu 0.99 -r 0.99 -B 10 -P 3 -n 20 
                #./warmup2 -lambda 2 -mu 0.35 -r 2 -B 10 -P 3 -n 20 -t tsfile.txt
                #./warmup2 -B 3 -t w2data/f0.txt > f0.out

                # echo "===> test_3 (minimum emulation time is 16.8 seconds)"
                # echo -n "     start time: "; date
                # ./warmup2 -r 4.5 -t w2data/f3.txt > f3.out
                # echo -n "       end time: "; date
                # ./analyze-trace.txt -r 4.5 -t w2data/f3.txt f3.out > f3.ana
                # ;;

                # echo "===> test_0 (minimum emulation time is 26.5 seconds)"
                # echo -n "     start time: "; date
                # ./warmup2 -B 3 -t w2data/f0.txt > f0.out
                # echo -n "       end time: "; date
                # ./analyze-trace.txt -B 3 -t w2data/f0.txt f0.out > f0.ana
                # ;;

                echo "===> test file"
                ./warmup2 -r 4.5 -t test_input.txt > test.out
                ;;

                make clean
