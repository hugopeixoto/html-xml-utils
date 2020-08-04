:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxref -l >$TMP1 <<-EOF
	<p lang=en><dfn>boss</dfn> is referenced twice:
	<span class="copy-this">bosses</span> and <em title="boss">here.</em>
EOF

# Add a newline at the end:
echo >>$TMP1

cat >$TMP2 <<-EOF
	<html><body><p lang="en"><dfn id="boss">boss</dfn> is referenced twice:
	<a href="#boss" class="copy-this">bosses</a> and <a href="#boss"><em title="boss">here.</em></a>
	</p></body></html>
EOF

cmp -s $TMP1 $TMP2
