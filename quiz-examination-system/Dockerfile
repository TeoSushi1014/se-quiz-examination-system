# Dockerfile for Quiz Examination System
# This is a development/deployment container for WinUI 3 C++/WinRT application

FROM mcr.microsoft.com/windows/servercore:ltsc2022

# Set working directory
WORKDIR /app

# Install Visual C++ Redistributables (required for C++ runtime)
ADD https://aka.ms/vs/17/release/vc_redist.x64.exe /temp/vc_redist.x64.exe
RUN /temp/vc_redist.x64.exe /install /quiet /norestart

# Install Windows App SDK Runtime (required for WinUI 3)
ADD https://aka.ms/windowsappsdk/1.8/latest/windowsappruntimeinstall-x64.exe /temp/windowsappruntime.exe
RUN /temp/windowsappruntime.exe /install /quiet /norestart

# Copy application files
COPY quiz-examination-system/x64/Release/ ./bin/

# Copy configuration files
COPY quiz-examination-system/*.config ./bin/

# Set environment variables
ENV SUPABASE_URL=https://your-project.supabase.co
ENV SUPABASE_ANON_KEY=your-anon-key

# Expose any necessary ports (if needed for web services)
# EXPOSE 8080

# Entry point - adjust based on your executable name
ENTRYPOINT ["bin\\quiz-examination-system.exe"]

# Health check (optional)
HEALTHCHECK --interval=30s --timeout=10s --retries=3 \
  CMD powershell -Command "if (Get-Process quiz-examination-system -ErrorAction SilentlyContinue) { exit 0 } else { exit 1 }"

# Metadata
LABEL maintainer="Software Engineering Course"
LABEL description="Quiz Examination System - WinUI 3 C++/WinRT Application"
LABEL version="1.0"
