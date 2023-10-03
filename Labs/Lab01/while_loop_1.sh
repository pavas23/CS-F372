# bash program to print all numbers from 1 to 100
num=1
while [ $num -le 100 ]
do
    echo $num
    let num=num+1
done