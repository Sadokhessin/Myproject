// Initialize Vue app
new Vue({
    el: '#app',
    data: {
      showConfigMenu: false,
      showMqttInputs: false,
      mqttServer: '',
      mqttTopic: ''
    },
    methods: {
      // Method to toggle config menu visibility
      toggleConfigMenu() {
        this.showConfigMenu = !this.showConfigMenu;
      },
      // Method to finish MQTT configuration
      finishMqttConfig() {
        // Here you can add logic to handle the MQTT configuration
        console.log('MQTT Server:', this.mqttServer);
        console.log('MQTT Topic:', this.mqttTopic);
        // Reset MQTT inputs and hide them
        this.mqttServer = '';
        this.mqttTopic = '';
        this.showMqttInputs = false;
      }
    }
  });