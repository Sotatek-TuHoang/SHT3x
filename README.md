Dự án ESP32 với Cảm biến SHT3x, Smart Config, MQTT và Chế độ Power Save

GIỚI THIỆU

Dự án này là một ứng dụng sử dụng vi điều khiển ESP32 và framework ESP-IDF để thu thập dữ liệu từ cảm biến nhiệt độ và độ ẩm SHT3x. Dữ liệu sau đó được gửi đến máy chủ MQTT thông qua giao thức MQTT. Dự án cũng tích hợp chế độ tiết kiệm năng lượng để giảm tiêu thụ năng lượng của ESP32.

TÍNH NĂNG

Đọc dữ liệu nhiệt độ và độ ẩm từ cảm biến SHT3x.
Gửi dữ liệu đọc được lên máy chủ MQTT.
Sử dụng Smart Config để cấu hình thông số kết nối WiFi.
Thực hiện chế độ Power Save để giảm tiêu thụ năng lượng.
Yêu cầu
Kit phát triển ESP32.
ESP-IDF Framework đã được cài đặt.
Cảm biến nhiệt độ và độ ẩm SHT3x.
Máy chủ MQTT để nhận và gửi dữ liệu.
Công cụ phát triển và môi trường phát triển ESP-IDF.

CÀI ĐẶT VÀ CẤU HÌNH

Sao chép mã nguồn từ thư mục src vào dự án của bạn sử dụng ESP-IDF.

Đảm bảo rằng bạn đã cài đặt các thư viện cần thiết và cấu hình WiFi trong tệp bee_wifi.c.

Cấu hình địa chỉ và thông số kết nối của máy chủ MQTT trong tệp main.c.

Tùy chỉnh các thiết lập chế độ Power Save trong tệp main.c nếu cần thiết.

SỬ DỤNG

Chạy dự án trên kit phát triển ESP32.
ESP32 sẽ thực hiện việc đọc dữ liệu từ cảm biến SHT3x và gửi dữ liệu lên máy chủ MQTT.
Kiểm tra dữ liệu trên máy chủ MQTT để theo dõi nhiệt độ và độ ẩm.
Gửi lệnh xuống hệ thống thông qua máy chủ MQTT.