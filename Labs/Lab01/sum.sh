# don't give spaces in between the a = 5 etc
# $ is mathematical environment to conduct mathematical operations
a=5
b=7
c=$((a+b))
echo $c
c=$($a+$b)
echo $c
c=$a+$b
echo $c
c=$(( $a + $b ))
echo $c
c=a+b
echo $c

