Radasm之流不能显示Courier New带中文不是因为字体本身, 而是它根据一个ascii 表决定TextOutA的StringSize, 中文被拆成2个字符画了两次, 所以是乱码, 需要改掉它的ascii表才行.

附带了两个补丁分别对应Radasm和MasmEd, 至少可以正确显示中文, 不保证其他功能正常.