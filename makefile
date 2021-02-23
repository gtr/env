env:
	gcc env.c -o env

test1:
	./env

test2:
	./env -i key1=val1 key2=val2 key3=val3

test3:
	./env key1=val1 key2=val2 key3=val3

test4:
	./env -i key1=val1 key2=val2 key3=val3 TZ=PST8PDT date

test5:
	./env key1=val1 key2=val2 key3=val3 TZ=PST8PDT date

test6:
	./env key1=val1 EDITOR=nano key2=val2 

test7:
	./env key1=val1 key2=val2 key3=val3 EDITOR=nano key4=val4 key5=val5 key6=val6

test8:
	./env -i key1=val1 key2=val2 key3=val3 EDITOR=vim key4=val4 key5=val5 key6=val6 EDITOR=emacsz

test9:
	./env key1=val1 key2=val2 key1=val3

test10:
	./env -i key1=val1 key2=val2 key3=val3 key1=val4 key1=val5

test11: 
	./env key1=val1 key2=val2 key3=val3 key1=val4  EDITOR=emacs EDITOR=vim TZ=PST8PDT date

clean:
	rm env
