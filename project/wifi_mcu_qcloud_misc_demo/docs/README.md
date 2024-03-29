# Qcloud 工程使用示例

## 配网方式
支持多种配网方式连接到腾讯云，例如 airkiss/softap/smartconfig以及直连等方式。

配网的示例代码参见 `app/usr_app.c`。

腾讯云物联网开发平台对当前产品也需要作相同的配网方式，例如物联网开发平台设置产品交互方式为 **airkiss** ，那么工程代码也需要设置为 **airkiss** 配网。

**注意**，当使用 airkiss 方式配网时，需要给编译器传递一个宏参数，如下图所示：

![airkiss xcloud](images/airkiss_macro.png)

这个宏的来源参见 `airkiss_def.h` 头文件。

## 示例

选用 `qcloud_iot_samples/scenarized/light_data_template_sample.c` 作为示例代码，其配网方式为 **airkiss**，对应的产品的配网二维码如下：

![airkiss配网二维码](images/tencent_qcloud_light_data_template_demo.png)
