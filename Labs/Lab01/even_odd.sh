# bash program to check if number is even or odd
a=5
if [ $(($a%2)) == 0 ] 
then
    echo "The number is even"
else
    echo "The number is odd"
fi