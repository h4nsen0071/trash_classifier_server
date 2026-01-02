#!/usr/bin/env python
"""
Script để chạy Flask server
"""

import os
import sys

# Add src to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from app import app
from src.config import Config


def main():
    """Run Flask server with Gunicorn (production) or Flask dev server (debug)"""
    config = Config('config.yaml')
    
    host = config.get('server.host', '0.0.0.0')
    port = config.get('server.port', 5000)
    debug = config.get('server.debug', False)
    workers = config.get('server.workers', 4)
    
    print(f"Starting Trash Classifier Server...")
    print(f"Host: {host}")
    print(f"Port: {port}")
    print(f"Workers: {workers}")
    print(f"Debug: {debug}")
    print("-" * 40)
    
    if debug:
        # Development mode - Flask dev server
        print("⚠️  Running in DEBUG mode (development only)")
        app.run(host=host, port=port, debug=True)
    else:
        # Production mode - Gunicorn (Linux only)
        import platform
        
        if platform.system() == 'Linux':
            # Linux production - Use Gunicorn
            import subprocess
            print("✅ Starting Gunicorn production server (Linux)")
            cmd = [
                'gunicorn',
                '--bind', f'{host}:{port}',
                '--workers', str(workers),
                '--timeout', '30',
                '--access-logfile', '-',
                '--error-logfile', '-',
                'app:app'
            ]
            subprocess.run(cmd)
        else:
            # Windows development - Flask dev server
            print("⚠️  Windows detected - Using Flask dev server")
            print("   (Gunicorn chỉ chạy trên Linux)")
            print("   Deploy lên EC2 Ubuntu để dùng Gunicorn production server")
            print("-" * 40)
            app.run(host=host, port=port, debug=False, threaded=True)


if __name__ == '__main__':
    main()
