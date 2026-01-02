"""
Configuration Loader Module
Load và validate config từ config.yaml
"""

import yaml
import os
from typing import Any, Optional


class Config:
    """
    Singleton class để load và quản lý configuration.
    
    Usage:
        config = Config('config.yaml')
        model_path = config.get('model.path')
    """
    
    _instance: Optional['Config'] = None
    _config: dict = {}
    
    def __new__(cls, config_path: str = 'config.yaml') -> 'Config':
        """Singleton pattern - chỉ tạo 1 instance"""
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._instance._load(config_path)
        return cls._instance
    
    def _load(self, config_path: str) -> None:
        """Load config từ YAML file"""
        if not os.path.exists(config_path):
            raise FileNotFoundError(f"Config file not found: {config_path}")
        
        with open(config_path, 'r', encoding='utf-8') as f:
            self._config = yaml.safe_load(f)
        
        self._validate()
    
    def _validate(self) -> None:
        """Validate required config keys"""
        required_keys = ['model.path', 'classes', 'bin_mapping']
        
        for key in required_keys:
            if self.get(key) is None:
                raise ValueError(f"Missing required config key: {key}")
    
    def get(self, key_path: str, default: Any = None) -> Any:
        """
        Lấy giá trị config theo key path.
        
        Args:
            key_path: Đường dẫn key (e.g., 'model.path', 'validation.cv_checks.blur_threshold')
            default: Giá trị mặc định nếu không tìm thấy
            
        Returns:
            Giá trị config hoặc default
        """
        keys = key_path.split('.')
        value = self._config
        
        try:
            for key in keys:
                value = value[key]
            return value
        except (KeyError, TypeError):
            return default
    
    def get_all(self) -> dict:
        """Trả về toàn bộ config"""
        return self._config.copy()
    
    @classmethod
    def reset(cls) -> None:
        """Reset singleton instance (dùng cho testing)"""
        cls._instance = None
        cls._config = {}
