#!/bin/sh

#
#0000000 190c 0000 6913 0000 7368 0000 144f 0000
#0000010 4771 0000 7368 0000 af63 0000 1ac5 0000
#0000020 744d 0000 56ff 0000 af63 0000 4d6b 0000
#0000030 af63 0000 839d 0000 8689 0000 a539 0000
#0000040 4d6b 0000 4452 0000 0b47 0000          
#000004c
#

#PS     因为加密后的数据中很有规律的出现很多0000，所以我在发送时去掉了无效的0000，只发送
#       有效数据，接收方接收到数据后会按照相同规则填充0000才能够正确的解析

if [ $# -eq 0 ]
then 
    echo "      格式：./script id ch rsad rsan"
    echo "          id:设备id，ch设备通信信道"
    echo "          rsad:rsa密钥中的d,rsan:rsa密钥中的n"
    echo "      该脚本会对efile文件内容进行更改，更改为“WM+OD+181130”+“当前时分秒”，并进行加密"
    echo "      输出十六进制的数据可以通过串口发送给单片机来进行测试。"
    return
fi

if [ -z $1 ]
then
    echo "please input device id"
    return 0
fi

if [ -z $2 ]
then
    echo "please input device ch"
    return
fi

if [ -z $3 ]
then
    echo "you must input rsa d"
    return
fi

if [ -z $4 ]
then
    echo "you must input rsa n"
    return
fi

id=$1
channel=$2
rsa_d=$3
rsa_n=$4

TMPFILE=.tmpfile
TXTFILE=1.txt

day=`date | awk -F " " '{print $4}' | awk -F ":" '{print $1$2$3}'
`
ctrlmsg="WM+OD+181130"${day}

echo ${ctrlmsg} > efile

./r -e efile ${rsa_d} ${rsa_n}

hexdump rsaencrypt.txt > ${TMPFILE}

#get main message 
cat ${TMPFILE} | awk -F " " '{print $2$4$6$8}' > ${TXTFILE}

#get total line number
line=`cat ${TMPFILE} | wc -l`

i=1
str=""
str1=""
while [ ${i} -ne ${line} ]
do
    #get total char numbers every line
    num=`cat ${TXTFILE} | sed -n ${i}p | wc -c`
    #截取的开始地址   
    start=1 
    #截取的结束地址
    end=2
    #当前窃取字符位置的奇偶标志
    flag=0
    
    while [ ${start} -ne ${num} ]
    do
        #提取出一个字符
        ch=`cat ${TXTFILE} | sed -n ${i}p | cut -c ${start}-${end}`
        #指向下一个字符   
        start=`expr ${start} + 2`
        end=`expr ${end} + 2`
        
        #后两个字节放到前面
        if [ `expr ${flag} % 2` -ne 0 ];
        then 
            str=${str}" "${ch}" "${evench}
            str1=${str1}" 0x"${ch}" 0x"${evench}
        fi
        #保存偶数位置字符
        evench=${ch}
        #标志位改变
        flag=`expr ${flag} + 1`
    done
    i=`expr ${i} + 1`
done

rm ${TMPFILE}
rm ${TXTFILE}

div=256
hh=`expr ${id} / ${div}`
ll=`expr ${id} % ${div}`
printf "%02x %02x %02x " ${hh} ${ll} ${channel} 
echo ${str}
