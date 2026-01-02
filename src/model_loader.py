"""
YOLO Model Loader Module
Singleton pattern để load và cache model
"""

from typing import Optional
from ultralytics import YOLO
import threading


class ModelLoader:
    """
    Singleton class để load và quản lý YOLO model.
    Thread-safe implementation.
    
    Usage:
        loader = ModelLoader(config)
        model = loader.get_model()
    """
    
    _instance: Optional['ModelLoader'] = None
    _model: Optional[YOLO] = None
    _lock = threading.Lock()
    
    def __new__(cls, config=None) -> 'ModelLoader':
        """Singleton pattern"""
        if cls._instance is None:
            with cls._lock:
                if cls._instance is None:
                    cls._instance = super().__new__(cls)
                    cls._instance._config = config
        return cls._instance
    
    def get_model(self) -> YOLO:
        """
        Lấy YOLO model (load nếu chưa có).
        
        Returns:
            YOLO model instance
        """
        if self._model is None:
            with self._lock:
                if self._model is None:
                    self._load_model()
        return self._model
    
    def _load_model(self) -> None:
        """Load YOLO model từ file"""
        model_path = self._config.get('model.path', 'models/best.pt')
        device = self._config.get('model.device', 'cpu')
        
        self._model = YOLO(model_path)
        
        # Set device
        if device != 'cpu':
            self._model.to(device)
    
    def reload_model(self) -> YOLO:
        """Force reload model"""
        with self._lock:
            self._model = None
            self._load_model()
        return self._model
    
    def is_loaded(self) -> bool:
        """Kiểm tra model đã load chưa"""
        return self._model is not None
    
    @classmethod
    def reset(cls) -> None:
        """Reset singleton (dùng cho testing)"""
        cls._instance = None
        cls._model = None
