# 日志报文结构
## magic number

## header
- header length
- encrypt
- content length
- log level
- 其他Key-Value

### 日志类型
1. api response
2. exception dump
3. crash dump

## content
1. 时间戳
2. 内容
3. 进程ID
4. 线程ID
5. Tag
6.