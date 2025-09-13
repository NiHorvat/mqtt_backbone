#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>



static const struct gpio_dt_spec indicator_led = GPIO_DT_SPEC_GET(DT_ALIAS(indicator_led), gpios);

static const struct gpio_dt_spec connected_led = GPIO_DT_SPEC_GET(DT_ALIAS(connected_led), gpios);


int init_indicator_led(){

    int err;
    
    if(!gpio_is_ready_dt(&indicator_led)){
        printk("%s indicator_led is not ready\n",__func__);
    }

    err = gpio_pin_configure_dt(&indicator_led, GPIO_OUTPUT);
    if(err < 0){
        printk("%s indicator_led failed to configure (err %d)\n",__func__, err);
        return err;
    }

    err = gpio_pin_set_dt(&indicator_led,0);
    if(err < 0){
        printk("%s indicator_led failed to set pin (err %d)\n",__func__, err);
        return err;
    }


    if(!gpio_is_ready_dt(&connected_led)){
        printk("%s indicator_led is not ready\n",__func__);
    }

    err = gpio_pin_configure_dt(&connected_led, GPIO_OUTPUT);
    if(err < 0){
        printk("%s indicator_led failed to configure (err %d)\n",__func__, err);
        return err;
    }

    err = gpio_pin_set_dt(&connected_led,0);
    if(err < 0){
        printk("%s indicator_led failed to set pin (err %d)\n",__func__, err);
        return err;
    }


    return 0;
}


void indicator_led_set_state(int new_state){

    int err = gpio_pin_set_dt(&indicator_led,new_state);
    if(err < 0){
        printk("%s indicator_led failed to set pin (err %d)\n",__func__, err);
    }

}



void connected_led_set_state(int new_state){

    int err = gpio_pin_set_dt(&connected_led,new_state);
    if(err < 0){
        printk("%s indicator_led failed to set pin (err %d)\n",__func__, err);
    }

}