:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxref -l >$TMP1 <<-EOF
	<p lang=en><dfn>term</dfn> is referenced twice:
	<span class="copy-this">term</span> and <em title="terms">plural.</em>
EOF

# Add a newline at the end:
echo >>$TMP1

cat >$TMP2 <<-EOF
	<html><body><p lang="en"><dfn id="term">term</dfn> is referenced twice:
	<a href="#term" class="copy-this">term</a> and <a href="#term"><em title="terms">plural.</em></a>
	</p></body></html>
EOF

cmp -s $TMP1 $TMP2
