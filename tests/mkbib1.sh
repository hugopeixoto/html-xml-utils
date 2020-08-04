:
trap 'rm $TMP1 $TMP2 $TMP3 $TMP4 $TMP5' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP4=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP5=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

# Make a Refer database with bibliographic data
#
cat >$TMP1 <<-EOF
	%L LABEL1
	%T Title One
	%A Author One

	%L LABEL2
	%T Title Two
	%A Author Two
	%A Another Author
	%D 2013

EOF

# Make an auxiliary file with a list of labels
#
cat >$TMP2 <<-EOF
	LABEL2
	LABEL2
	LABEL1
EOF

# Make a template for the generated bibliography
#
cat >$TMP3 <<-EOF
	Bibliography

	# %L sorted by label

	%{L:%L
	  %A
	  %T
	  %{D:%D%}%{!D:no date%}

	%}End
EOF

# The expected output of hxmkbib
#
cat >$TMP4 <<-EOF
	Bibliography

	#  sorted by label

	LABEL1
	  Author One
	  Title One
	  no date

	LABEL2
	  Author Two; Another Author
	  Title Two
	  2013

	End
EOF

# Run hxmkbib with the above three files
#
./hxmkbib -a $TMP2 $TMP1 $TMP3 >$TMP5

# Compare to the expected output
#
cmp -s $TMP4 $TMP5
