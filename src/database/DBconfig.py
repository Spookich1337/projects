from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession, async_sessionmaker
from src.database.DBmodels import *


DATABASE_URL = "postgresql+asyncpg://admin:admin@postgres:5432/mydb"
engine = create_async_engine(
    DATABASE_URL,
    pool_size=5, 
    max_overflow=10,
    pool_pre_ping=True,
)


AsyncSessionLocal = async_sessionmaker(
    engine,
    class_=AsyncSession,
    expire_on_commit=False,
)


async def get_db():
    async with AsyncSessionLocal() as session:
        try:
            yield session
        finally:
            await session.close()