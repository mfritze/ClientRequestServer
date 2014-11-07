for i in {1..100}
do
	(wget localhost:$1/large.html &)
done