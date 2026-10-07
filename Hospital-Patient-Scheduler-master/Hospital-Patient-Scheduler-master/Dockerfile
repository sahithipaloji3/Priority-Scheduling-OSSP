FROM maven:3.9-eclipse-temurin-21 AS build

RUN apt-get update && \
    apt-get install -y gcc && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY web-java/pom.xml web-java/pom.xml
COPY web-java/src web-java/src

COPY src src

RUN gcc -O2 \
    src/web_scheduler.c \
    src/scheduler.c \
    src/metrics.c \
    src/fileio.c \
    -o web_scheduler

WORKDIR /app/web-java

RUN mvn clean package -DskipTests

FROM eclipse-temurin:21-jre

WORKDIR /app

COPY --from=build /app/web-java/target/hospital-patient-scheduler-1.0.0.jar app.jar
COPY --from=build /app/web_scheduler web_scheduler

RUN chmod +x /app/web_scheduler

EXPOSE 8080

CMD ["java", "-jar", "app.jar"]
