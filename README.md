# Trash Classifier Server - IoT Smart Bin

Flask REST API phân loại rác thông minh sử dụng YOLO11.

## 🎯 Tính năng

- **3 loại rác**: Paper (Thùng 1), Plastic (Thùng 2), Glass (Thùng 3)
- **CV Validation**: Kiểm tra blur, brightness, hand detection
- **AI Classification**: YOLO11 inference
- **Modular Architecture**: Dễ maintain và mở rộng

## 📁 Cấu trúc

```
trash_classifier_server/
├── app.py                  # Main Flask app
├── config.yaml             # Configuration
├── models/best.pt          # YOLO model
├── src/
│   ├── config.py           # Config loader
│   ├── model_loader.py     # YOLO singleton
│   ├── validators/         # CV checks
│   ├── classifiers/        # AI inference
│   ├── utils/              # Utilities
│   └── api/                # Endpoints
└── tests/                  # Tests
```

## 🚀 Cài đặt

```bash
pip install -r requirements.txt
```

## ▶️ Chạy server

```bash
python app.py
```

Server chạy tại: `http://localhost:5000`

## 📡 API Endpoints

### POST /classify
Phân loại rác từ ảnh.

**Request:**
```bash
curl -X POST -F "image=@test.jpg" http://localhost:5000/classify
```

**Response (Success):**
```json
{
  "success": true,
  "bin": 1,
  "class": "paper",
  "confidence": 0.95,
  "processing_time_ms": 150
}
```

**Response (Invalid):**
```json
{
  "success": false,
  "bin": 0,
  "reason": "Image too blurry",
  "failed_check": "blur_check",
  "processing_time_ms": 50
}
```

### GET /health
Kiểm tra server status.

```json
{
  "status": "healthy",
  "model_loaded": true,
  "timestamp": "2024-12-30T10:30:00"
}
```

## 🔧 Configuration

Chỉnh sửa `config.yaml`:

```yaml
validation:
  cv_checks:
    blur_threshold: 100      # Ngưỡng blur
    confidence_threshold: 0.70  # Ngưỡng AI
```

## 🧪 Testing

```bash
pytest tests/
```

## 🐳 Docker

```bash
docker build -t trash-classifier .
docker run -p 5000:5000 trash-classifier
```

## 📝 License

MIT License - IoT Smart Bin Project
