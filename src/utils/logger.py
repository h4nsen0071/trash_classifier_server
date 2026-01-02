"""
Logger Module
Custom logger cho Flask server
"""

import logging
import os
from datetime import datetime
from typing import Optional


class Logger:
    """
    Singleton logger class với file và console handlers.
    """
    
    _instance: Optional['Logger'] = None
    _logger: Optional[logging.Logger] = None
    
    def __new__(cls, name: str = 'trash_classifier', 
                log_dir: str = 'logs',
                level: int = logging.INFO) -> 'Logger':
        """Singleton pattern"""
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._instance._setup(name, log_dir, level)
        return cls._instance
    
    def _setup(self, name: str, log_dir: str, level: int) -> None:
        """Setup logger với handlers"""
        self._logger = logging.getLogger(name)
        self._logger.setLevel(level)
        
        # Tránh duplicate handlers
        if self._logger.handlers:
            return
        
        # Format
        formatter = logging.Formatter(
            '%(asctime)s - %(name)s - %(levelname)s - %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )
        
        # Console handler
        console_handler = logging.StreamHandler()
        console_handler.setLevel(level)
        console_handler.setFormatter(formatter)
        self._logger.addHandler(console_handler)
        
        # File handler
        try:
            os.makedirs(log_dir, exist_ok=True)
            log_file = os.path.join(log_dir, f'{name}_{datetime.now():%Y%m%d}.log')
            file_handler = logging.FileHandler(log_file, encoding='utf-8')
            file_handler.setLevel(level)
            file_handler.setFormatter(formatter)
            self._logger.addHandler(file_handler)
        except Exception as e:
            self._logger.warning(f"Could not create file handler: {e}")
    
    def debug(self, message: str) -> None:
        """Log debug message"""
        self._logger.debug(message)
    
    def info(self, message: str) -> None:
        """Log info message"""
        self._logger.info(message)
    
    def warning(self, message: str) -> None:
        """Log warning message"""
        self._logger.warning(message)
    
    def error(self, message: str) -> None:
        """Log error message"""
        self._logger.error(message)
    
    def critical(self, message: str) -> None:
        """Log critical message"""
        self._logger.critical(message)
    
    def log_request(self, endpoint: str, method: str, client_ip: str) -> None:
        """Log incoming request"""
        self.info(f"Request: {method} {endpoint} from {client_ip}")
    
    def log_classification(self, result: dict, duration_ms: float) -> None:
        """Log classification result"""
        class_name = result.get('class', 'unknown')
        confidence = result.get('confidence', 0)
        bin_num = result.get('bin', 0)
        self.info(f"Classification: {class_name} ({confidence:.2%}) -> Bin {bin_num} [{duration_ms:.0f}ms]")
    
    def log_validation_error(self, errors: list) -> None:
        """Log validation errors"""
        for error in errors:
            self.warning(f"Validation failed: {error}")
    
    @classmethod
    def reset(cls) -> None:
        """Reset singleton (dùng cho testing)"""
        if cls._logger:
            cls._logger.handlers = []
        cls._instance = None
        cls._logger = None


# Global logger instance
logger = Logger()
