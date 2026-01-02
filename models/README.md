# Trash Classifier Server - Models Directory

Đặt file model YOLO (best.pt) vào thư mục này.

## Cấu trúc:
```
models/
├── README.md
└── best.pt      <- Copy model đã train vào đây
```

## Lấy model từ Colab:
1. Sau khi train xong trên Colab, download file `best.pt` từ `/content/runs/detect/train/weights/best.pt`
2. Copy file vào thư mục `models/` này
3. Cập nhật đường dẫn trong `config.yaml` nếu cần

## Lưu ý:
- File model thường có kích thước 5-50MB tùy theo loại YOLO
- Không commit model vào git (đã thêm vào .gitignore)
