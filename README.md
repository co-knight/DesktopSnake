# Desktop Snake (C/WinAPI)

**Note: This game only works on `Windows 10`.** Microsoft modified the desktop API in `Windows 11`.

Features:
- Uses existing desktop icons for gameplay
- Automatically detects desktop icon grid size
- Disables and re-enables "Align icons to Grid" for smooth animation
- Prevents 180-degree turns

Controls:
- W / Up Arrow:    Move Up
- S / Down Arrow:  Move Down
- A / Left Arrow:  Move Left
- D / Right Arrow: Move Right
- ESC:             Exit Game

How to compile (using the provided Makefile):
```
make
```

## Inspiration

[Can Snake be implemented in C within 100 lines? - Answer by 仗键天涯 on Zhihu](
https://www.zhihu.com/question/360814879/answer/3617660687)

For aesthetic and readability purposes, the code is not intentionally compressed. If you can optimize the code to reduce the number of lines, PRs are welcome and greatly appreciated.

---

中文介绍

# 桌面贪吃蛇 (C/WinAPI)

**注意，该游戏只对 `Windows 10` 有效。** 因为微软在 `Windows 11` 修改了桌面API。

特色:
- 使用现有的桌面图标实现有效。
- 自动检测桌面图标网格大小。
- 禁用和重新启用"对齐到网格"以实现平滑动画。
- 防止180度转弯。

控制:
- W / 上箭头:   向上
- S / 下箭头:   向下
- A / 左箭头:   向左
- D / 右箭头:  向右
- ESC:              退出游戏

如何编译（使用提供的Makefile）:
```
make
```

## 灵感来源

[用C语言，能在100行之内实现贪吃蛇吗？ - 仗键天涯的回答 - 知乎](
https://www.zhihu.com/question/360814879/answer/3617660687)

出于美观和便于阅读考虑，没有特意去压行。如果有大佬能优化代码缩短行数，欢迎提交PR，十分感谢。