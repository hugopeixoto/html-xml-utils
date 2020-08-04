:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<-EOF
	<p class=index>Not indexed 1
	<span class=index>Indexed 1</span>
	<em class=index>Not indexed 2</em>
	<i class=index>Indexed 2</i>
	<p>Not indexed 3
	<span class=index>Indexed 3</span>
	<!-- begin-index -->
	<p>Remove this.
	<!-- end-index -->
	EOF

LC_ALL=C ./hxindex -t -X p,em $TMP1 >$TMP2
echo >>$TMP2

cat >$TMP3 <<-EOF
	<html><body><p class="index">Not indexed 1
	<span id="indexed-1" class="index">Indexed 1</span>
	<em class="index">Not indexed 2</em>
	<i id="indexed-2" class="index">Indexed 2</i>
	</p><p>Not indexed 3
	<span id="indexed-3" class="index">Indexed 3</span>
	<!--begin-index-->
	<ul class="indexlist">
	<li>Indexed 1, <a href="#indexed-1">#</a>
	<li>Indexed 2, <a href="#indexed-2">#</a>
	<li>Indexed 3, <a href="#indexed-3">#</a>
	</ul><!--end-index--></p></body></html>
	EOF

cmp -s $TMP2 $TMP3
