:
# Illegal UTF-8 sequence

echo -e "\0340\0200\0274" | ./xml2asc >/dev/null
[[ $? != 0 ]]
