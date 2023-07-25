# esp32_idf_aws
ESP32 AWS IDF Example

# setup
1. 참고 aws iot 예제 
> https://github.com/espressif/esp-aws-iot

2. 사용하는 IDF Version의 examples 폴더에서 아래 명령을 이용하여 copy 한다.
> git clone -b "release/202210.01-LTS" --recursive https://github.com/espressif/esp-aws-iot

3. vscode의 "ESP-IDF: Show Examples Projects" 을 통해 "tls_mutual_auth" 예제를 생성한다.

4. CMakeLists.txt 에서 libraries path을 아래와 같이 수정한다.
```
set(EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/examples/common_components/protocol_examples_common"
						 "$ENV{IDF_PATH}/examples/esp-aws-iot/libraries/backoffAlgorithm"
						 "$ENV{IDF_PATH}/examples/esp-aws-iot/libraries/coreMQTT"
						 "$ENV{IDF_PATH}/examples/esp-aws-iot/libraries/common/posix_compat")
```

 # sdkconfig.defaults 설정
- 로그를 시간 베이스로 확인 하기 위한 설정
```
CONFIG_LWIP_DHCP_GET_NTP_SRV=y
CONFIG_LOG_TIMESTAMP_SOURCE_SYSTEM=y
```
- WIFI 설정
```
CONFIG_EXAMPLE_WIFI_SSID="TwinsMesh"
CONFIG_EXAMPLE_WIFI_PASSWORD="01020114473"
```
- AWS 설정
```
CONFIG_MQTT_CLIENT_IDENTIFIER="test"
CONFIG_MQTT_BROKER_ENDPOINT="ajwyncrxuu7ba-ats.iot.us-west-2.amazonaws.com"
```

# 인증서 관련 
- aws site에서 iot 생성시 다운 받은 인증서를 셋팅 한다.(파일 내용을 copy paste 한다. )
> ![image](https://github.com/firepooh/esp32_idf_aws/assets/19420294/83a2524c-5c51-4334-9943-0002d9c6935f)

c2a877a658beba55b3118342d7b9d1cc254f0aa9918d50d641bca80cc26aaf35-certificate.pem.crt --> client.crt
c2a877a658beba55b3118342d7b9d1cc254f0aa9918d50d641bca80cc26aaf35-private.pem.key     --> client.key
