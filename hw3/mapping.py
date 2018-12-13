#! /usr/bin/python
'''
    File name: tranfer_Big5_ZhuYin.py
    Author: leo.chang
    Date created: 12/10/2018
    Date last modified: 12/10/2018
    Python Version: 3.6
'''

import sys

Big5_ZhuYin = sys.argv[1]
f_in = open(Big5_ZhuYin, 'r', encoding='BIG5-HKSCS')

big5_dicList = {}

for line in  f_in:
    line = line.split(' ')
    big5 = line[0]
    # print(big5_dicList)
    big5_dicList[big5] = [big5]
    zhuyin = line[1].split('/')
    for sep in zhuyin:
        # print(big5_dicList)
        zhuyin = line[1].split('/')
        if sep[0] in big5_dicList:
            # print('big5' +  big5, 'sep' +  sep[0], big5_dicList)
            big5_dicList[sep[0]].append(big5)
        else:
            big5_dicList[sep[0]] = [big5]

ZhuYin_Big5 = sys.argv[2]
f_out = open(ZhuYin_Big5, 'w', encoding='BIG5-HKSCS')
for word in sorted(big5_dicList):
    line = word + ' ' + ' '.join(big5_dicList[word]) + '\n'
    f_out.write(line)
f_in.close()
f_out.close()
