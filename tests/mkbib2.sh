:

trap 'rm $TEMPLATE $BIBFILE $AUXFILE $TMP1 $TMP2 $TMP3' 0
TEMPLATE=`mktemp /tmp/tmp-XXXXXX` || exit 1
BIBFILE=`mktemp /tmp/tmp-XXXXXX` || exit 1
AUXFILE=`mktemp /tmp/tmp-XXXXXX` || exit 1
TMP1=`mktemp /tmp/tmp-XXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp-XXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp-XXXXXX` || exit 1

cat >$TEMPLATE <<EOF
<html>
<title>Bibliography</title>
<p>... text with [[Java]] here...
<!--%A%D sorted on author, then date -->
<dl>
%{L:
<dt id="%L">%{A:%A%}%{!A:%{E:%E%}%{!E:%{Q:%Q%}%{!Q:-%}%}%}</dt>
<dd>%{B:"%T"
  in: %{E:%E (eds)
  %}<cite>%B.</cite>%{V: %V.%}
  %}%{J:"%T"
  in: %{E:%E (eds)
  %}<cite>%J.</cite>%{V: %V.%}%{N: %N.%}%{P: pp. %P.%}
  %}%{!B:%{!J:<cite>%T.</cite>
  %}%}%{I:%I.
  %}%{D:%D.
  %}%{C:%C.
  %}%{R:%R.
  %}%{S:%S.
  %}%{O:%O
  %}%{U:<a href="%U">%U</a>
  %}</dd>
%}
</dl>
</html>
EOF

cat >$BIBFILE <<EOF
%L Java
%A Gosling, James
%A Joy, Bill
%A Steele, Guy
%T The Java language specification
%D 1998
%I Addison-Wesley
%U http://java.sun.com/docs/books/jls/index.html
EOF

cat >$TMP1 <<EOF
<html>
<title>Bibliography</title>
<p>... text with <a href="#Java" rel="biblioentry">[Java]<!--{{Java}}--></a> here...
<!-- sorted on author, then date -->
<dl>

<dt id="Java">Gosling, James; Joy, Bill; Steele, Guy</dt>
<dd><cite>The Java language specification.</cite>
  Addison-Wesley.
  1998.
  <a href="http://java.sun.com/docs/books/jls/index.html">http://java.sun.com/docs/books/jls/index.html</a>
  </dd>

</dl>
</html>
EOF

./hxcite -a $AUXFILE $BIBFILE $TEMPLATE | ./hxmkbib -a $AUXFILE $BIBFILE >$TMP2

diff -u $TMP1 $TMP2
