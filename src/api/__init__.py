"""
API Blueprint modules
"""

from .classify import classify_bp
from .health import health_bp

__all__ = [
    'classify_bp',
    'health_bp'
]
