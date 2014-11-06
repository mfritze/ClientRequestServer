for i in {1..100}
do
	(wget localhost:$1/file.html &)
done