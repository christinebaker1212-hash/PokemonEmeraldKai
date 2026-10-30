#!/bin/bash
# item_desc: the mart's box and the PC's ITEM STORAGE box both turned to their second page
n=$(grep -c "\[test\] itemdesc: font 7, 5 lines, 3 shown, top 3, paging 1" "$1/a.log")
[ "$n" -ge 2 ] && echo "item_desc ok ($n second pages)" || { echo "second pages: $n (want the mart's and the PC's)"; exit 1; }
