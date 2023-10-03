# bash program to print all numbers from 100 to 0
num=100
while [ $num -ge 0 ]
do  
    echo $num
    let num=num-1
done
