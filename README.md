# NS3
Network communication technology of NUAA
实验包括两个部分：一模拟星型无线网络、二模拟一个多跳网络，并分析吞吐量。
星型无线网络包含5个接入点，一个AP，多跳网络包括6个节点。
分析吞吐量首先用gawk分析运行.cc文件产生的trace文件，提取出数据组，然后用gnuplot画图画出，需要注意的是，在用gawk分析的时候，要根据实际情况修改.awk脚本中的程序。
