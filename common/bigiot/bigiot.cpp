#include "bigiot.h"

#if 1
#define BIG_DEBUG(...) uart1.printf("\n[BIGIOT]:");uart1.printf(__VA_ARGS__)
#define BIG_RECV(...) uart1.printf("\n[BIG RECV]:");uart1.printf(__VA_ARGS__)
#else
#define BIG_DEBUG(...)
#define BIG_RECV(...) 
#endif

bool BigIot::connect(String *remote_ip, uint32_t remote_port, uint16_t local_port)
{
    uint32_t last_time = millis();
    uint8_t buf[512]={0};
    uint16_t len = 0;
    bool ret;
    
    ret = BigIotPort::connect(remote_ip,remote_port,local_port);
    BIG_DEBUG("TCP connecting...");

    
    return ret;

}    

bool BigIot::login(String *device_id,String *apikey)
{

    bool ret;
    uint16_t len = 0;
    uint8_t buf[512]={0};
    uint32_t last_time = millis();
    String msg="{\"M\":\"checkin\",\"ID\":\"" + *device_id + "\",\"K\":\"" + *apikey + "\"}\n";
    ret = send((uint8_t*)msg.c_str(),msg.length());
    BIG_DEBUG("LOGIN:%s",msg.c_str());
    BIG_DEBUG("LOGIN......");
    
    return online;

}
bool BigIot::logout(String *device_id,String *apikey)
{

    bool ret;
    uint32_t last_time = millis();
    String msg="{\"M\":\"checkout\",\"ID\":\""+*device_id+"\",\"K\":\""+*apikey+"\"}\n";
    BIG_DEBUG("logout......");
    ret = send((uint8_t*)msg.c_str(),msg.length());
    BIG_DEBUG("LOGOUT:%s",msg.c_str());
    delay_ms(500);
    disconnect();
    delay_ms(500);
    while(connected());
    if( (!connected()))
    {
        online = false;
        BIG_DEBUG("logout success!");
        
    }
   
    return online;

}
bool BigIot::realtime_data(String *device_id,String *data_id,uint32_t val)
{
    bool ret;
    String VAL(val);
    String str = "{\"M\":\"update\",\"ID\":" + *device_id + ",\"V\":{\"" + *data_id + "\":" + VAL + "}}\n";
    ret = send((uint8_t *)str.c_str(), str.length());
    BIG_DEBUG("real time data:%s",str.c_str());
    if(ret){
        return true;
    }
    else{
        return false;
    }
}

bool BigIot::get_server_time(String *date_time)
{
    bool ret;
    String str = "{\"M\":\"time\",\"F\":\"Y-m-d H:i:s\"}\n";
    ret = send((uint8_t *)str.c_str(), str.length());
    BIG_DEBUG("get server time:%s",str.c_str());
    if(ret){
        return true;

    }
    else{
        return false;
    }
}
bool BigIot::active_alert(String *msg,BIGIOT_ALERT_TYPE type)
{
    bool ret;
    String _type;
    switch((uint8_t)type)
    {
        case BIGIOT_EMAIL:
            _type = "email";
            break;
        case BIGIOT_WEIBO:
            _type = "weibo";
            break;
        case BIGIOT_QQ:
            _type = "qq";
            break;
        default:
            _type = "email";
            break;   
    }
    String str = "{\"M\":\"alert\",\"C\":\""+*msg+"\",\"B\":\""+_type+"}\n";
    ret = send((uint8_t *)str.c_str(), str.length());
    BIG_DEBUG("get server time:%s",str.c_str());
    if(ret){
        return true;

    }
    else{
        return false;
    }

}
bool BigIot::quarry_status()
{
    bool ret;
    String str = "{\"M\":\"status\"}\n";
    ret = send((uint8_t *)str.c_str(), str.length());
    BIG_DEBUG("quarry status!");
    if(ret){
        return true;

    }
    else{
        return false;
    }

}
bool BigIot::quarry_is_online(String *id_list)
{
    bool ret;
    String str = "{\"M\":\"isOL\",\"ID\":[\""+ *id_list +"\"]}\n";
    ret = send((uint8_t *)str.c_str(), str.length());
    BIG_DEBUG("get server time:%s",str.c_str());
    if(ret){
        return true;

    }
    else{
        return false;
    }

}


bool BigIot::say(BIGIOT_USER_TYPE type,String *id,String *msg,String *sign)
{
    bool ret;
    String str;
    
    if(sign != NULL){
        String _sign(*sign);
        str = "{\"M\":\"say\",\"ID\":\""+ *id +"\",\"C\":\""+ *msg +"\",\"SIGN\":\""+*sign+"\"}\n";
    }else{
        switch((uint8_t)type)
        {
            case BIGIOT_ALL:
                str = "{\"M\":\"say\",\"ID\":\"ALL\",\"C\":\""+ *msg +"\"}\n";break;
            case BIGIOT_USER:
                str = "{\"M\":\"say\",\"ID\":\"U"+ *id +"\",\"C\":\""+ *msg +"\"}\n";break;
            case BIGIOT_DEVICE:
                str = "{\"M\":\"say\",\"ID\":\"D"+ *id +"\",\"C\":\""+ *msg +"\"}\n";break;
            case BIGIOT_GUSET:
                str = "{\"M\":\"say\",\"ID\":\"G"+ *id +"\",\"C\":\""+ *msg +"\"}\n";break;
        }
    }
   ret = send((uint8_t *)str.c_str(), str.length());
    BIG_DEBUG("SAY:%s",str.c_str());
    if(ret){
        return true;
    }
    else{
        return false;
    }
}
void BigIot::process_message(uint8_t *buf)
{
    uint16_t len=0;
    
    len = read_until(buf,'\n');
    if(len > 0){
        buf[len -1 ] = '\0';
//        String str=(const char *)buf;
        //BIG_RECV("==%s==",(const char *)buf);
//        if(str.startsWith("{") && str.endsWith("}")){
            cJSON * pJson =cJSON_Parse((const char*)buf);
            cJSON * method = cJSON_GetObjectItem(pJson, "M");
            String M = method->valuestring;
            BIG_DEBUG("M:[%s]",M.c_str());
            if(M == "say"){
                cJSON * content = cJSON_GetObjectItem(pJson, "C");
                cJSON * client_id = cJSON_GetObjectItem(pJson, "ID");
                String C = content->valuestring;
                String F_C_ID = client_id->valuestring;
                if(C == "play"){
                    PB8.reset();
                }else if(C == "stop"){
                    PB8.set();
                }
                cJSON_Delete(content);
                cJSON_Delete(client_id);
            }
            if(M == "time")
            {
                cJSON * content = cJSON_GetObjectItem(pJson, "T");
                String T = content->valuestring;
                BIG_DEBUG(T.c_str());
            
            }
            if(M == "checkinok"){
                BIG_DEBUG("device online------------------------");
                online = true;
            }
            cJSON_Delete(pJson);
            cJSON_Delete(method);
        }
//    }
    
}